

#include "../Includes/Cgi_handler.hpp"

// Execute the CGI script in the child process

void Cgi::setupCgiPipes()
{
    if (hasRequestBody())
        setupCgiStdin();

    redirectCgiOutput();
}

bool Cgi::hasRequestBody()
{
    return !Context.clientRequests[client_fd].get_filename().empty();
}

void Cgi::setupCgiStdin()
{
    // Open the file containing the request body
    const std::string &bodyFile = Context.clientRequests[client_fd].get_filename();
    cgi_stdin = open(bodyFile.c_str(), O_RDONLY);
    if (cgi_stdin == -1)
    {
        close(cgi_stdout);
        throw std::runtime_error("Failed to open request body file for CGI stdin");
    }

    setNonBlockCloexec(cgi_stdin);

    redirectCgiInput();
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
        _exit(1);
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
    const std::map<std::string, std::string> &params = Context.clientRequests[client_fd].getQueryParams();

    for (std::map<std::string, std::string>::const_iterator it = params.begin(); it != params.end(); ++it)
    {
        std::string envVar = it->first + "=" + it->second;
        env.push_back(strdup(envVar.c_str()));
    }
    env.push_back(NULL);
    return env;
}

void Cgi::runExecve(const char *interpreter, const std::vector<char*> &args, std::vector<char*> &env)
{
    if (execve(interpreter, const_cast<char* const*>(args.data()), env.data()) == -1)
    {
        _exit(1);
    }
}

void Cgi::executeCgiScript()
{
    setupCgiPipes();
    std::cout << "--CGI Environment built for client: " << client_fd << std::endl;

    Response &response = Context.clientResponses[client_fd];
    const char *scriptInterpreter = response.getCurrentRoute()->cgi_Path_Info.c_str();
    std::string scriptPath = response.getPath();

    std::vector<char*> env = buildCgiEnv();
    std::vector<char*> args = buildCgiArgs(scriptPath);

    runExecve(scriptInterpreter, args, env);
}
