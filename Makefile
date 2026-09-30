PROJECTS	= clang ld

.PHONY: all clean $(PROJECTS)

all: $(PROJECTS)
	@echo "Build complete"

fmt:
	@find . -name "*.c" -o -name "*.h" | xargs clang-format -i

clean:
	@@for dir in $(PROJECTS); do    \
		$(MAKE) -C $$dir clean;     \
	done

$(PROJECTS):
	$(MAKE) -C $@
