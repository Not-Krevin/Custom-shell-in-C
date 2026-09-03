
CC := gcc
CFLAGS := -std=c17 -Wall -Wextra -Wpedantic -Werror -g
TARGET := minish
SRC := src/minish.c

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

test: $(TARGET)
	bash tests/smoke.sh

clean:
	rm -f $(TARGET)
