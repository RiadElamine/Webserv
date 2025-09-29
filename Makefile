NAME = Webserver

SRCS_DIR = ./srcs
HEADERS_DIR = ./Includes
SOURCE = $(shell find srcs -name "*.cpp")
OBJECT = $(SOURCE:.cpp=.o)

HEADER = $(shell find Includes -name "*.hpp")

CFLAGS = -Wall -Wextra -Werror -std=c++98 -g

all : $(NAME)

$(NAME): $(OBJECT)
	c++ $(CFLAGS) $(OBJECT) -o $(NAME)

%.o: %.cpp $(HEADER)
	c++ $(CFLAGS) -c $<  -o $@

clean:
	rm -f $(OBJECT)

fclean: clean
	rm -f $(NAME)

re: fclean all
