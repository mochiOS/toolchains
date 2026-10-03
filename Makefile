PROJECTS	= clang ld
VERSION		?= 0.1.0
ARCH		?= x86_64
DIST		= out/dist
RELEASE_DIR	= $(DIST)/$(ARCH)-mochios-toolchain-$(VERSION)
ARCHIVE		= $(DIST)/$(ARCH)-mochios-toolchain-$(VERSION).tar.zst

.PHONY: all fmt clean test release release-stage release-clean $(PROJECTS)

all: fmt $(PROJECTS)
	@echo "Build complete"

fmt:
	@find . -name "*.c" -o -name "*.h" | xargs clang-format -i

clean:
	@@for dir in $(PROJECTS); do    \
		$(MAKE) -C $$dir clean;     \
	done

$(PROJECTS):
	$(MAKE) -C $@

release: release-clean all release-stage
	@tar -C $(DIST) \
		-cf - $(ARCH)-mochios-toolchain-$(VERSION) \
		| zstd -19 -T0 -o $(ARCHIVE)
	@cd $(DIST) && \
		sha256sum $(notdir $(ARCHIVE)) > SHA256SUMS
	@echo "Release created: $(ARCHIVE)"

release-stage:
	@mkdir -p $(RELEASE_DIR)/bin

	@install -m 755 \
		clang/out/$(ARCH)-mochios-clang \
		$(RELEASE_DIR)/bin/$(ARCH)-mochios-clang

	@install -m 755 \
		ld/out/$(ARCH)-mochios-ld \
		$(RELEASE_DIR)/bin/$(ARCH)-mochios-ld

	@install -m 644 \
		license \
		$(RELEASE_DIR)/LICENSE

release-clean:
	@rm -rf $(DIST)
