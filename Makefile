# Native build (tests, player libraries) and WebAssembly build (`make wasm`).
CC      ?= cc
CFLAGS  ?= -Wall -std=c99 -O2 -g
LDLIBS  := -lm

BUILD := build
SRV   := src/server
PLY   := src/players
TST   := src/test

HDRS     := $(wildcard $(SRV)/*.h $(PLY)/*.h)
BITBOARD := $(SRV)/bitboard.c
COMMON   := $(PLY)/matrix.c $(PLY)/list.c

TESTS := test_bitboard

.PHONY: all test clean doc

all: test

$(BUILD):
	mkdir -p $@

$(BUILD)/test_bitboard: $(TST)/test_bitboard.c $(BITBOARD) $(HDRS) | $(BUILD)
	$(CC) $(CFLAGS) $(filter %.c,$^) -o $@ $(LDLIBS)

test: $(addprefix $(BUILD)/,$(TESTS))
	@set -e; for t in $(TESTS); do echo "== $$t"; ./$(BUILD)/$$t; done

doc:
	cd doc/doxygen && doxygen Doxyfile

clean:
	rm -rf $(BUILD) doc/doxygen/html
