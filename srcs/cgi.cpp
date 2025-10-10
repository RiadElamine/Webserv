#include "../Includes/response.hpp"

// void parseCGIheader(std::string& buffer, Response& response) {

// }


// void makeCGIbofy(std::string& buffer, Response& response) {

// }

// std::string readCGI(std::string filename) {
// 	Response response;
// 	std::string buffer = readFile(filename);
// 	parseCGIheader(buffer, response);
// 	makeCGIbofy(buffer, response);

// 	return response.getResponse()

// }


void executeCGI(std::string outFile, char* args[]) {
    int fd = open(outFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
        throw std::runtime_error("Error while opening the file");
    int saved_stdout = dup(1);        // Save original stdout
    dup2(fd, 1);                      // Redirect stdout to file
    close(fd);                        // fd no longer needed

    pid_t pid = fork();
    if (pid == 0) {
        execve(args[0], args, NULL);
        std::cerr << "error " << std::endl;
        exit(1);
    } else if (pid == -1) {
        throw std::runtime_error("Can't create child process");
    }
    waitpid(pid, NULL, 0);

    // Restore stdout
    dup2(saved_stdout, 1);
    close(saved_stdout);
}
