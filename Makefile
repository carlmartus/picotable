CC ?= gcc
CFLAGS ?= -Wall -Wextra -Werror -Wpedantic -Wshadow -Wunused-parameter -std=c11 -I.

# Test build
TEST_SRC = tests/test_picotable.c
TEST_BIN = tests/test_picotable

# Sample build
SAMPLE_SRC = sample/products.c sample/fruit_counter.c
SAMPLE_BIN = sample/products sample/fruit_counter

.PHONY: test clean help samples

# Build and run tests
test: $(TEST_BIN)
	@./$(TEST_BIN)

# Build test binary
$(TEST_BIN): $(TEST_SRC) picotable.h
	$(CC) $(CFLAGS) -o $@ $< -lcriterion -lpthread

# Build sample binaries
sample/products: sample/products.c picotable.h
	$(CC) $(CFLAGS) -o $@ $< -lncursesw

sample/fruit_counter: sample/fruit_counter.c picotable.h
	$(CC) $(CFLAGS) -o $@ $<

# Clean
clean:
	rm -f $(TEST_BIN) $(SAMPLE_BIN)

# Build samples
samples: $(SAMPLE_BIN)

# Show test command
help:
	@echo "Make targets:"
	@echo "  test    - Build and run tests"
	@echo "  samples - Build sample programs"
	@echo "  clean   - Remove build artifacts"
	@echo "  help    - Show this help message"

