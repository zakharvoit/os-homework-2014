ifeq ($(TARGET), debug)
CFLAGS = -fPIC -O0 -g -Wall -Wextra
else
CFLAGS = -fPIC -O2 -Wall -Wextra
endif

LIBRARY = libhelpers.so
OBJ = helpers.o
SRC = $(OBJ:%.o=%.c)

TEST = run_tests
TEST_CFLAGS = $(CFLAGS) -L. -Wl,-rpath=. -lhelpers

all: depend $(LIBRARY)

$(LIBRARY): $(OBJ)
	$(CC) $(CFLAGS) -shared -o $@ $^

depend: .depend

.depend: $(SRC)
	-rm -f .depend
	$(CC) $(CFLAGS) -MM $^ -MF .depend

test: $(TEST)
	./$(TEST)

$(TEST): $(TEST).c $(LIBRARY)
	$(CC) $(TEST_CFLAGS) -o $@ $^

-include .depend

clean:
	-rm -f $(LIBRARY) $(OBJ)
	-rm -f $(TEST)
	-rm -f .depend

.PHONY: all clean depend test
