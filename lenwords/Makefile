LIBHELPERS_PATH = ../lib

CFLAGS = -O2 -Wall -Wextra -I$(LIBHELPERS_PATH)
LDFLAGS = -lhelpers -Wl,-rpath=$(LIBHELPERS_PATH) -L$(LIBHELPERS_PATH)

OUTPUT = lenwords
OBJ = lenwords.o
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

.PHONY: all clean depend
