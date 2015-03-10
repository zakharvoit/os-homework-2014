LIBRARY = libhelpers.so
OBJ = helpers.o
SRC = $(OBJ:%.o=%.c)

CFLAGS = -fPIC -O2 -Wall -Wextra

all: depend $(LIBRARY)

$(LIBRARY): $(OBJ)
	$(CC) $(CFLAGS) -shared -o $@ $^

depend: .depend

.depend: $(SRC)
	-rm -f .depend
	$(CC) $(CFLAGS) -MM $^ -MF .depend

include .depend

clean:
	-rm -f $(LIBRARY) $(OBJ)
	-rm -f .depend

.PHONY: all clean depend
