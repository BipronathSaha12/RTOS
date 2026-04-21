CC       = gcc
CFLAGS   = -Wall -Wextra -Werror -std=c11 -g -O2 -Iinclude
LDFLAGS  = -lpthread -lrt

SRC_DIR  = src
BUILD_DIR = build
INC_DIR  = include

SRCS     = $(wildcard $(SRC_DIR)/*.c)
OBJS     = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
TARGET   = $(BUILD_DIR)/rtos_sim

.PHONY: all clean run rebuild

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "  [LINK] $@"
	@$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "  Build complete: $@"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(wildcard $(INC_DIR)/*.h)
	@mkdir -p $(BUILD_DIR)
	@echo "  [CC]   $<"
	@$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	@echo ""
	@./$(TARGET)

clean:
	@echo "  Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)/*.o $(TARGET)

rebuild: clean all
