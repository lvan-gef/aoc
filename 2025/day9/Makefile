NAME      := main

CC        := clang
CFLAGS    := -std=c17 -Wall -Wextra -Werror -Wshadow -Wpedantic -Wconversion -Wdouble-promotion
R_FLAGS   := -DNDEBUG -O3 -march=native -fomit-frame-pointer

SRC_DIR   := src
OBJ_DIR   := obj

SRC_FILES := main.c

SRCS      := $(addprefix $(SRC_DIR)/, $(SRC_FILES))
OBJECTS   := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

BLUE      := \033[36m
MARGENTA  := \033[35m
NC        := \033[0m

.PHONY: all
all: $(NAME)  ## Build release version (default)

.PHONY: run
run: all  ## Build release and run
	./$(NAME) input.txt

.PHONY: clean
clean:  ## Clean object files
	@rm -rf $(OBJ_DIR)
	@echo "Cleaned object files"

.PHONY: fclean
fclean: clean  ## Clean object, bin
	@rm -f $(NAME)
	@echo "Cleaned everything"

.PHONY: re
re: fclean all  ## Clean all and recompile

.PHONY: fmt
fmt:  ## Format code via clang-format
	@echo "Format code"
	@find . -type f -name "*.c" -print0 | xargs -0 clang-format -i

.PHONY: help
help:  ## Get help
	@echo -e 'Usage: make ${BLUE}<target>${NC}'
	@echo -e 'Available targets:'
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "  ${BLUE}%-15s${NC} %s\n", $$1, $$2}' $(MAKEFILE_LIST)

$(NAME): $(OBJECTS)
	$(CC) $(OBJECTS) -s -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(R_FLAGS) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)
