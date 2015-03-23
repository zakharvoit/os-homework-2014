SUBMODULES = lib cat revwords

SUBMODULES_RULE = $(SUBMODULES:%=%.submodule)

all: $(SUBMODULES_RULE)

%.submodule:
	@echo "Making $(@:%.submodule=%)"
	@$(MAKE) --no-print-directory -C $(@:%.submodule=%)

clean:
	@for dir in $(SUBMODULES) ; do \
		echo "Cleaning $$dir"; \
		$(MAKE) --no-print-directory clean -C $$dir; \
	done

.PHONY: all clean
