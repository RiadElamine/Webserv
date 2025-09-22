NAME=res
SRC= $(shell find src -name "*.cpp")
INCLUDE= $(shell find src -name "*.hpp")
CXXFLAGS=-Wall -Wextra -Werror -std=c++98
OBJ= $(SRC:.cpp=.o)
# CC=/home/oel-asri/.local/bin/c++
CC=c++

all: $(NAME)

$(NAME): $(OBJ)
	@echo "Linking Object Files"
	@$(CC) $(CXXFLAGS) $(OBJ) -o $@
%.o: %.cpp $(INCLUDE)
	@echo "Compiling Source Files"
	@$(CC) $(CXXFLAGS) $< -c -o $@
run: $(NAME)
	./$(NAME)
clean:
	@echo "Removing Objects Files"
	@rm -rf $(OBJ)
fclean: clean
	@echo "Deleting Program Binary"
	@rm -rf $(NAME)
re: fclean all
	@echo "Rebuilding Project Done."

