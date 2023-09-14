
TARGET = main


CC = gcc
CFLAGS = -Wall -Wextra -Werror -g -fsanitize=address
SOURCES = mini_serv.c
OBJECTS = $(SOURCES:.c=.o)
all: $(TARGET)

$(TARGET): $(OBJECTS)
	@$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)
	@rm -f $(OBJECTS)
	@./$(TARGET) 1479


%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -f $(OBJECTS) $(TARGET)
