CC ?= gcc
CFLAGS ?= -Wall -Wextra -Werror -Wpedantic -Wshadow -Wunused-parameter -std=c11 -I.

# Test build
TEST_SRC = tests/test_picotable.c
TEST_BIN = tests/test_picotable

.PHONY: test clean help

# Build and run tests
test: $(TEST_BIN)
	@./$(TEST_BIN)

# Build test binary
$(TEST_BIN): $(TEST_SRC) picotable.h
	$(CC) $(CFLAGS) -o $@ $< -lcriterion -lpthread

# Clean
clean:
	rm -f $(TEST_BIN)

# Show test command
help:
	@echo "Make targets:"
	@echo "  test    - Build and run tests"
	@echo "  clean   - Remove build artifacts"
	@echo "  help    - Show this help message"

