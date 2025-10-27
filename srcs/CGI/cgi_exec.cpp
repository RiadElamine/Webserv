

#include "../../Includes/CGI/Cgi.hpp"
#include "../../Includes/Request/HttpRequest.hpp"

// Execute the CGI script in the child process

void Cgi::changeToCgiDirectory() {

    if (chdir(cgi_dir.c_str()) == -1) {
        std::cerr << "chdir failed: " << std::strerror(errno) << std::endl;
        std::exit(1);
    }
}

void Cgi::setupCgiPipes()
{
    if (hasRequestBody())
        setupCgiStdin();

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
    cgi_stdin = open(bodyFile.c_str(), O_RDONLY);
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

std::vector<char*> Cgi::buildCgiEnv()
{
    std::vector<char*> env;
    const std::map<std::string, std::string> &params = this->params;
    char *envVar;

    for (std::map<std::string, std::string>::const_iterator it = params.begin(); it != params.end(); ++it)
    {
        std::string params_str = it->first + "=" + it->second;
        envVar = strdup(params_str.c_str());
        if (!envVar)
        {
            // Handle memory allocation failure
            for (size_t i = 0; i < env.size(); ++i)
                free(env[i]);
            env.clear();
            exit(1);
        }
        env.push_back(envVar);
    }

    HttpRequest &request = *Context->clientRequests[client_fd];
    std::map<std::string, std::string> headers = request.getHeaders();
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
    {
        std::string headerVar = it->first + "=" + it->second;
        envVar = strdup(headerVar.c_str());
        if (!envVar)
        {
            // Handle memory allocation failure
            for (size_t i = 0; i < env.size(); ++i)
                free(env[i]);
            env.clear();
            exit(1);
        }
        env.push_back(envVar);  // Fixed: was calling strdup again, causing memory leak
    }

    if (!request.getMethod().empty())
    {
        std::string requestMethodVar = "REQUEST_METHOD=" + request.getMethod();
        envVar = strdup(requestMethodVar.c_str());
        if (!envVar)
        {
            // Handle memory allocation failure
            for (size_t i = 0; i < env.size(); ++i)
                free(env[i]);
            env.clear();
            exit(1);
        }
        env.push_back(envVar);
    }
    if (request.getContentLength() > 0)
    {
        std::stringstream contentLengthVar;
        contentLengthVar << "CONTENT_LENGTH=" << request.getContentLength();
        envVar = strdup(contentLengthVar.str().c_str());
        if (!envVar)
        {
            // Handle memory allocation failure
            for (size_t i = 0; i < env.size(); ++i)
                free(env[i]);
            env.clear();
            exit(1);
        }
        env.push_back(envVar);
    }
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
    // If execve succeeds, the process memory is replaced, so no need to free
}

void Cgi::executeCgiScript()
{
    if (cgi_stdout != -1) {
        close(cgi_stdout);
        cgi_stdout = -1;  // Reset to avoid double closing
    }
    
    changeToCgiDirectory();

    setupCgiPipes();
    
    std::cout << "In child process to execute CGI script" << std::endl;

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

    runExecve(scriptInterpreter, args, env);
}
