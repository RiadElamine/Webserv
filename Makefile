NAME = tests

SRCS_DIR = ./srcs
HEADERS_DIR = ./Includes
SOURCE =  $(SRCS_DIR)/main.cpp  $(SRCS_DIR)/ConfigFile.cpp $(SRCS_DIR)/Webserver.cpp
OBJECT = $(SOURCE:.cpp=.o)

HEADER = $(HEADERS_DIR)/ConfigFile.hpp $(HEADERS_DIR)/Webserver.hpp 

CFLAGS = -Wall -Wextra -Werror -std=c++98

all : $(NAME)

$(NAME): $(OBJECT)
	c++ $(CFLAGS) $(OBJECT) -o $(NAME)

$(SRCS_DIR)/%.o: $(SRCS_DIR)/%.cpp $(HEADER)
	c++ $(CFLAGS) -c $<  -o $@

clean:
	rm -f $(OBJECT)

fclean: clean
	rm -f $(NAME)

re: fclean all
