LIBHELPERS_PATH = ../lib

ifeq ($(TARGET), debug)
CFLAGS = -D_GNU_SOURCE -D_POSIX_SOURCE -std=c11 -g -O0 -DDEBUG -D_GLIBC_DEBUG -Wall -Wextra -I$(LIBHELPERS_PATH)
else
CFLAGS = -D_GNU_SOURCE -D_POSIX_SOURCE -std=c11 -O2 -Wall -Wextra -I$(LIBHELPERS_PATH)
endif

LDFLAGS = -Wl,-rpath=$(LIBHELPERS_PATH) -L$(LIBHELPERS_PATH)
LDLIBS = -lhelpers -lbufio

FORKING = forking
FORKING_OBJ = forking.o
POLLING = polling
POLLING_OBJ = polling.o
SRC = $(FORKING_OBJ:%.o=%.c) $(POLLING_OBJ:%.o=%.c)

all: depend $(FORKING) $(POLLING)

test:
	./test.sh

$(FORKING): $(FORKING_OBJ)

depend: .depend

.depend: $(SRC)
	-rm -f .depend
	$(CC) $(CFLAGS) -MM $^ -MF .depend

-include .depend

clean:
	-rm -f $(FORKING) $(POLLING) $(FORKING_OBJ) $(POLLING_OBJ)
	-rm -f .depend

.PHONY: all clean depend test
