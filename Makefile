SUBMODULES = lib cat revwords filter bufcat foreach simplesh filesender forking
export CFLAGS = -std=c11 -Wall -Wextra

SUBMODULES_RULE = $(SUBMODULES:%=%.submodule)

all: $(SUBMODULES_RULE)

%.submodule:
	@echo "=== Making $(@:%.submodule=%) ==="
	@$(MAKE) --no-print-directory -C $(@:%.submodule=%)

clean:
	@for dir in $(SUBMODULES) ; do \
		echo "=== Cleaning $$dir ==="; \
		$(MAKE) --no-print-directory clean -C $$dir; \
	done

.PHONY: all clean
