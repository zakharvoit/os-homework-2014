LIBBUFIO_PATH = ../lib

CFLAGS = -O2 -Wall -Wextra -I$(LIBBUFIO_PATH)
LDFLAGS = -L$(LIBBUFIO_PATH)  -Wl,-rpath=$(LIBBUFIO_PATH)
LDLIBS = -lbufio

OUTPUT = bufcat
OBJ = bufcat.o
SRC = $(OBJ:%.o=%.c)

all: depend $(OUTPUT)

$(OUTPUT): $(OBJ)

test: $(OUTPUT)
	./test.sh

depend: .depend

.depend: $(SRC)
	-rm -f .depend
	$(CC) $(CFLAGS) -MM $^ -MF .depend

-include .depend

clean:
	-rm -f $(OUTPUT) $(OBJ)
	-rm -f .depend

.PHONY: all clean depend test
