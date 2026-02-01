CC = gcc
CFLAGS = -Wall -Wextra -std=c99 $(shell sdl2-config --cflags) -Iinclude
LIBS = -lm $(shell sdl2-config --libs)

SRC_DIR = src
BUILD_DIR = build
TARGET = $(BUILD_DIR)/program

# Source files in new directory structure
SRCS = $(SRC_DIR)/main.c \
       $(SRC_DIR)/core/linked_list.c \
       $(SRC_DIR)/core/math_utils.c \
       $(SRC_DIR)/core/profiler.c \
       $(SRC_DIR)/physics/collision.c \
       $(SRC_DIR)/physics/forces.c \
       $(SRC_DIR)/physics/integrator.c \
       $(SRC_DIR)/spatial/grid.c \
       $(SRC_DIR)/spatial/particle_factory.c \
       $(SRC_DIR)/render/renderer.c

OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $@

# Pattern rules for different directory depths
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/core/%.o: $(SRC_DIR)/core/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/physics/%.o: $(SRC_DIR)/physics/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/spatial/%.o: $(SRC_DIR)/spatial/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/render/%.o: $(SRC_DIR)/render/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)/core $(BUILD_DIR)/physics $(BUILD_DIR)/spatial $(BUILD_DIR)/render

clean:
	rm -rf $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET)
