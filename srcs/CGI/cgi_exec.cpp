

#include "../../Includes/CGI/Cgi.hpp"
#include "../../Includes/Request/HttpRequest.hpp"

// Execute the CGI script in the child process

void Cgi::changeToCgiDirectory() {
    if (chdir(cgi_dir.c_str()) == -1) {
std::cerr << "hreee " << cgi_dir << std::endl;
        std::cerr << "chdir failed: " << std::strerror(errno) << std::endl;
        std::exit(1);
    }
}

void Cgi::setupCgiPipes()
{
    filename = Context->clientRequests[client_fd]->get_filename();
    if (hasRequestBody())
    {
        setupCgiStdin();

    }

    setupCgiStdout();
}

bool Cgi::hasRequestBody()
{
    return !filename.empty();
}

void Cgi::setupCgiStdin()
{
    // Open the file containing the request body
    const std::string &bodyFile = filename;
    cgi_stdin = open(bodyFile.c_str(),  O_RDONLY);
    if (cgi_stdin == -1)
    {
        perror("open body file for cgi_stdin");
        close(cgi_stdout);
        exit(1);
    }

    setNonBlockCloexec(cgi_stdin);

    redirectCgiInput();
}

void Cgi::setupCgiStdout()
{
    cgi_stdout = open(filename_cgi_output.c_str(), O_WRONLY);
    if (cgi_stdout == -1){
        perror("open cgi_stdout");
        exit(1);
    }

    setNonBlockCloexec(cgi_stdout);

    redirectCgiOutput();
}

void Cgi::redirectCgiInput()
{
    if (dup2(cgi_stdin, STDIN_FILENO) == -1)
    {
        close(cgi_stdin);
        exit(1);
    }
}

void Cgi::redirectCgiOutput()
{
    if (dup2(cgi_stdout, STDOUT_FILENO) == -1)
    {
        close(cgi_stdout);
        close(cgi_stdin);
        exit(1);
    }
}

std::vector<char*> Cgi::buildCgiArgs(const std::string &scriptPath)
{
    std::vector<char*> args;
    args.push_back(const_cast<char*>("myprogram"));
    args.push_back(const_cast<char*>(scriptPath.c_str()));
    args.push_back(NULL);
    return args;
}

void addEnvVar(std::vector<char*>& env, const std::string& var)
{
    char* envVar = strdup(var.c_str());
    if (!envVar)
    {
        // Handle memory allocation failure
        for (size_t i = 0; i < env.size(); ++i)
            free(env[i]);
        env.clear();
        std::cerr << "Memory allocation failed in addEnvVar()\n";
        exit(1);
    }
    env.push_back(envVar);
}

void addParamsToEnv(std::vector<char*>& env, const std::map<std::string, std::string>& params)
{
    std::string param_str = "QUERY_STRING=";

    for (std::map<std::string, std::string>::const_iterator it = params.begin(); it != params.end(); ++it)
    {
        param_str += it->first + "=" + it->second + "&";
    }

    if (!params.empty()) {
        param_str.pop_back(); 
    }

    addEnvVar(env, param_str); 
}

void Cgi::updateCGIEnvironment(std::vector<char*>& env, const HttpRequest &req) 
{
    std::map<std::string, std::string> headers = req.getHeaders();

    // 1. Set CGI-required static variables
    addEnvVar(env, "GATEWAY_INTERFACE=CGI/1.1");
    addEnvVar(env, "SERVER_PROTOCOL=HTTP/1.1");
    addEnvVar(env, "SERVER_SOFTWARE=WebServer/1.0");

    // 2. Headers
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
    {
        std::string key = it->first;
        std::string value = it->second;

        std::transform(key.begin(), key.end(), key.begin(), ::toupper);
        std::replace(key.begin(), key.end(), '-', '_');

        if (key == "CONTENT_LENGTH" || key == "CONTENT_TYPE")
        {
            addEnvVar(env, key + "=" + value);
        }
        else if (key == "HOST")
        {
            std::string host = value;
            std::string port = "80";
            std::string::size_type pos = value.find(':');
            if (pos != std::string::npos)
            {
                host = value.substr(0, pos);
                port = value.substr(pos + 1);
            }
            addEnvVar(env, "SERVER_NAME=" + host);
            addEnvVar(env, "SERVER_PORT=" + port);
        }
        else if (key == "AUTHORIZATION")
        {
            std::string authType = value;
            std::string::size_type space = value.find(' ');
            if (space != std::string::npos)
                authType = value.substr(0, space);
            addEnvVar(env, "AUTH_TYPE=" + authType);
        }
        else
        {
            addEnvVar(env, "HTTP_" + key + "=" + value);
        }
  
        if (!req.getMethod().empty())
            addEnvVar(env, "REQUEST_METHOD=" + req.getMethod());

        if (!path.empty())
            addEnvVar(env, "SCRIPT_NAME=" + path);

    }
}

std::vector<char*> Cgi::buildCgiEnv()
{
    std::vector<char*> env;

    addParamsToEnv(env,  this->params);

    // 
    HttpRequest &request = *Context->clientRequests[client_fd];
    updateCGIEnvironment(env, request);

    env.push_back(NULL);
    return env;
}

void Cgi::runExecve(const char *interpreter, const std::vector<char*> &args, std::vector<char*> &env)
{
    if (execve(interpreter, const_cast<char* const*>(args.data()), env.data()) == -1)
    {
        perror("execve");
        exit(1);
    }
}

void Cgi::executeCgiScript()
{
    if (cgi_stdout != -1) {
        close(cgi_stdout);
        cgi_stdout = -1; 
    }
    
    setupCgiPipes();

    // Determine the script interpreter based on file extension
    std::string ext = path.substr(path.find_last_of('.'));
    std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
    const char *scriptInterpreter = currentLocation->cgi_Path_Info[ext].c_str();

    // Remove cgi_dir prefix if present
    std::string scriptPath = path;
    if (scriptPath.find(cgi_dir) == 0) {
        scriptPath.erase(0, cgi_dir.length());
    }

    std::vector<char*> env = buildCgiEnv();
    std::vector<char*> args = buildCgiArgs(scriptPath);

    changeToCgiDirectory();

    runExecve(scriptInterpreter, args, env);
}
