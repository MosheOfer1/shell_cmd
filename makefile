# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -Werror -g

# Sources
SRCS = key_shell.c shell.c utilty.c

# Headers
HDRS = key_shell.h

# Object files
OBJS = $(SRCS:.c=.o)

# Executable
EXEC = myshell

# Default target
all: $(EXEC)

# Rule to build the executable
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Rule to build object files
%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean up
clean:
	rm -f $(OBJS) $(EXEC)

# Phony targets
.PHONY: all clean
