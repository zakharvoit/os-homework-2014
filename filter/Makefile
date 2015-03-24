LIBHELPERS_PATH = ../lib

ifeq ($(TARGET), debug)
CFLAGS = -g -O0 -D_GLIBC_DEBUG -Wall -Wextra -I$(LIBHELPERS_PATH)
else
CFLAGS = -O2 -Wall -Wextra -I$(LIBHELPERS_PATH)
endif

LDFLAGS = -lhelpers -Wl,-rpath=$(LIBHELPERS_PATH) -L$(LIBHELPERS_PATH)

OUTPUT = filter
OBJ = filter.o
SRC = $(OBJ:%.o=%.c)

all: depend $(OUTPUT)

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
