LIBHELPERS_PATH = ../lib

ifeq ($(TARGET), debug)
CFLAGS = -DRUN_ON_EMPTY -g -O0 -D_GLIBC_DEBUG -Wall -Wextra -I$(LIBHELPERS_PATH)
else
CFLAGS = -DRUN_ON_EMPTY -O2 -Wall -Wextra -I$(LIBHELPERS_PATH)
endif

LDFLAGS = -Wl,-rpath=$(LIBHELPERS_PATH) -L$(LIBHELPERS_PATH)
LDLIBS = -lhelpers -lbufio

OUTPUT = foreach
OBJ = foreach.o
SRC = $(OBJ:%.o=%.c)

all: depend $(OUTPUT)

test:
	./test.sh

$(OUTPUT): $(OBJ)

depend: .depend

.depend: $(SRC)
	-rm -f .depend
	$(CC) $(CFLAGS) -MM $^ -MF .depend

-include .depend

clean:
	-rm -f $(OUTPUT) $(OBJ)
	-rm -f .depend

.PHONY: all clean depend test
