ifeq ($(TARGET), debug)
CFLAGS = -std=c11 -DDEBUG -fPIC -O0 -g -Wall -Wextra
else
CFLAGS = -std=c11 -fPIC -O2 -Wall -Wextra
endif

LIB = lib$(NAME).so
OBJ = $(SRC:%.c=%.o)
DEP_NAME = .depend_$(LIB).mk

all: depend $(LIB)

$(LIB): $(OBJ)
	$(CC) $(CFLAGS) -shared -o $@ $^

depend: $(DEP_NAME)

$(DEP_NAME): $(SRC)
	-rm -f $(DEP_NAME)
	$(CC) $(CFLAGS) -MM $^ -MF $(DEP_NAME)

-include $(DEP_NAME)

clean:
	-rm -f $(LIB) $(OBJ)
	-rm -f $(DEP_NAME)

.PHONY: all clean depend
