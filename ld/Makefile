include config.mk

CC			= 	cc
STD			= 	c99
SRC			= 	src/main.c
OUT			= 	out
TARGET		= 	$(OUT)/$(ARCH)-mochios-ld
TEST_SRC	= 	tests/test.c
TEST_OBJ	= 	$(OUT)/test.o
TEST_BIN	= 	$(OUT)/test
CFLAGS		= 	-std=$(STD) -Wall -Wextra -Werror -O2 \
				-D_POSIX_C_SOURCE=200809L \
				-D$(ARCH)

.PHONY: all build clean

all: build

build:
	@mkdir -p $(OUT)
	@$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	@rm -rf $(OUT)

test: build
	@clang --target=x86_64-elf \
		-ffreestanding \
		-mno-red-zone \
		-fno-pie \
		-fno-stack-protector \
		-c $(TEST_SRC) \
		-o $(TEST_OBJ)
	@$(TARGET) --mochios-sdk "$(SDK)" $(TEST_OBJ) -o $(TEST_BIN)
	@test -f $(TEST_BIN)
	@echo "test passed"
