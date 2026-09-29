include config.mk

CC			= 	cc
STD			= 	c99
SRC			= 	src/main.c
OUT			= 	out
TARGET		= 	$(OUT)/$(ARCH)-mochios-clang
TEST_SRC	= 	tests/test.c
TEST_OBJ	= 	$(OUT)/test.o
CFLAGS		= 	-std=$(STD) -Wall -Wextra -Werror -O2 \
				-D_POSIX_C_SOURCE=200809L \
				-D$(ARCH)

.PHONY: all build test clean

all: build

build:
	@mkdir -p $(OUT)
	@$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

test: build
	@$(TARGET) -c $(TEST_SRC) -o $(TEST_OBJ)
	@test -f $(TEST_OBJ)
	@echo "test passed"

clean:
	@rm -rf $(OUT)
