NAME = webserv

SRCS_DIR = ./srcs
HEADERS_DIR = ./Includes

SOURCE = srcs/CGI/cgi.cpp \
         srcs/CGI/cgi_events.cpp \
         srcs/CGI/cgi_exec.cpp \
         srcs/CGI/cgi_parser.cpp \
         srcs/CGI/cgi_utils.cpp \
         srcs/Client.cpp \
         srcs/Request/HttpRequest.cpp \
         srcs/Response/fetchData.cpp \
         srcs/Response/response.cpp \
         srcs/Server/ConfigFile.cpp \
         srcs/Server/Webserver.cpp \
         srcs/brain.cpp \
         srcs/main.cpp \
         srcs/utils.cpp

OBJECT = $(SOURCE:.cpp=.o)

HEADER = Includes/CGI/Cgi.hpp \
         Includes/Client.hpp \
         Includes/Common.hpp \
         Includes/Request/HttpRequest.hpp \
         Includes/Response/response.hpp \
         Includes/Server/ConfigFile.hpp \
         Includes/Server/Webserver.hpp \
         Includes/brain.hpp \
         Includes/status_code.hpp \
         Includes/utils.hpp

CFLAGS = -Wall -Wextra -Werror -std=c++98 -g -fsanitize=address

all : $(NAME)

$(NAME): $(OBJECT)
	c++ $(CFLAGS) $(OBJECT) -o $(NAME)

%.o: %.cpp $(HEADER)
	c++ $(CFLAGS) -c $<  -o $@

clean:
	rm -f $(OBJECT)

fclean: clean
	rm -f $(NAME)
run : all clean
	./$(NAME)

re: fclean all
