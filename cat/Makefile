LIBHELPERS_PATH = ../lib

CFLAGS = -O2 -Wall -Wextra -I$(LIBHELPERS_PATH)
LDFLAGS =  -L$(LIBHELPERS_PATH) -Wl,-rpath=$(LIBHELPERS_PATH) -lhelpers 

OUTPUT = cat
OBJ = cat.o
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
