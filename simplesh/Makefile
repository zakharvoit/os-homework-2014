LIBHELPERS_PATH = ../lib

CFLAGS = -g -std=c11 -O2 -Wall -Wextra -I$(LIBHELPERS_PATH)
LDFLAGS =  -L$(LIBHELPERS_PATH) -Wl,-rpath=$(LIBHELPERS_PATH)
LDLIBS = -lhelpers -lbufio

OUTPUT = simplesh
OBJ = simplesh.o
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
