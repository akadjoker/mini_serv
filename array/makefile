NAME = main
FLAGS = -g -Wall -Wextra #-fsanitize=address
CC = clang
SRC = main.c

all:
	$(CC) $(FLAGS) $(SRC) -o $(NAME)
	@./$(NAME) 1478
	@rm -f  $(NAME)