

#include "../../Includes/CGI/Cgi.hpp"

// Execute the CGI script in the child process

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
    cgi_stdout = open("cgi_output.txt", O_CREAT | O_RDWR | O_TRUNC, 0644);
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
        exit(1);
    }
}

void Cgi::executeCgiScript()
{
    setupCgiPipes();
        // std::cout << "In child process to execute CGI script" << std::endl;

    const char *scriptInterpreter = currentLocation->cgi_Path_Info.c_str();
    std::string scriptPath = path;

    std::vector<char*> env = buildCgiEnv();
    std::vector<char*> args = buildCgiArgs(scriptPath);

    runExecve(scriptInterpreter, args, env);
}
