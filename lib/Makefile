all: libbufio.so libhelpers.so

MAKE = @make --no-print-directory

BUFIO_SRC = bufio.c
MAKE_BUFIO = $(MAKE) SRC="$(BUFIO_SRC)" NAME=bufio -f make_library.mk

HELPERS_SRC = helpers.c
MAKE_HELPERS = $(MAKE) SRC="$(HELPERS_SRC)" NAME=helpers -f make_library.mk

libbufio.so:
	$(MAKE_BUFIO)

libhelpers.so:
	$(MAKE_HELPERS)

TEST = run_tests
TEST_CFLAGS = $(CFLAGS) -L. -Wl,-rpath=. -lhelpers

test: $(TEST)
	./$(TEST)

$(TEST): $(TEST).c $(HELPERS)
	$(CC) $(TEST_CFLAGS) -o $@ $^

clean:
	$(MAKE_BUFIO) clean
	$(MAKE_HELPERS) clean

.PHONY: all clean test libbufio.so libhelpers.so
