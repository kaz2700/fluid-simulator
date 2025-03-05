# Variables
CC = gcc
CFLAGS = $(shell sdl2-config --cflags)   # Includes SDL2 compilation flags
LIBS = $(shell sdl2-config --libs)       # Includes SDL2 linking flags
SRC = main.c particle.c            # List all source files here
OBJ = $(SRC:.c=.o)                       # Create a list of object files
TARGET = program                          # Output target executable

# Default rule to build the program
all: $(TARGET)

# Linking rule to create the executable
$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(LIBS) -o $(TARGET)

# Compiling rule to convert .c files to .o files
$(OBJ): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule to remove compiled files
clean:
	rm -f $(OBJ) $(TARGET)
