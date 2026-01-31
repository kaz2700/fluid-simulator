# Variables
CC = gcc
CFLAGS = $(shell sdl2-config --cflags)   # Includes SDL2 compilation flags
LIBS = -lm $(shell sdl2-config --libs)       # Includes math library and SDL2 linking flags
SRC = main.c particle.c artist.c physics.c arraylist.c space_partition.c math_functions.c
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
