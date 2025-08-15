NAME = tests

SRCS_DIR = ./srcs
SOURCE =  $(SRCS_DIR)/main.cpp  $(SRCS_DIR)/ConfigFile.cpp
OBJECT = $(SOURCE:.cpp=.o)

HEADER = ./Includes/ConfigFile.hpp

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
