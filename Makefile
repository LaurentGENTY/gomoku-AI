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

TESTS := test_bitboard test_player test_moves test_matrix

.PHONY: all test players valgrind clean doc

all: test players

$(BUILD):
	mkdir -p $@

$(BUILD)/test_bitboard: $(TST)/test_bitboard.c $(BITBOARD) $(HDRS) | $(BUILD)
	$(CC) $(CFLAGS) $(filter %.c,$^) -o $@ $(LDLIBS)

$(BUILD)/test_player: $(TST)/test_player.c $(PLY)/heuristic2.c $(COMMON) $(BITBOARD) $(HDRS) | $(BUILD)
	$(CC) $(CFLAGS) $(filter %.c,$^) -o $@ $(LDLIBS)

$(BUILD)/test_moves: $(TST)/test_moves.c $(SRV)/moves.c $(BITBOARD) $(HDRS) | $(BUILD)
	$(CC) $(CFLAGS) $(filter %.c,$^) -o $@ $(LDLIBS)

$(BUILD)/test_matrix: $(TST)/test_matrix.c $(COMMON) $(HDRS) | $(BUILD)
	$(CC) $(CFLAGS) $(filter %.c,$^) -o $@ $(LDLIBS)

# The original players, built as in the project (one shared library each).
# Nothing loads them any more; building them checks the sources still compile.
PLAYERS := player4 player4.2 player4.3 player4.4

players: $(PLAYERS:%=$(BUILD)/%.so)

$(BUILD)/%.so: | $(BUILD)
	$(CC) $(CFLAGS) -fPIC -shared -pthread $(filter %.c,$^) -o $@ $(LDLIBS)

$(BUILD)/player4.so: $(PLY)/player4.c $(PLY)/heuristic0.c $(COMMON) $(HDRS)
$(BUILD)/player4.2.so: $(PLY)/player4.2.c $(PLY)/heuristic.c $(COMMON) $(BITBOARD) $(HDRS)
$(BUILD)/player4.3.so: $(PLY)/player4.3.c $(PLY)/heuristic2.c $(COMMON) $(BITBOARD) $(HDRS)
$(BUILD)/player4.4.so: $(PLY)/player4.4.c $(PLY)/heuristic3.c $(COMMON) $(BITBOARD) $(HDRS)

# Memory errors only: the original player code is not leak-free.
VALGRIND_TESTS := test_bitboard test_player test_moves test_matrix

valgrind: $(addprefix $(BUILD)/,$(VALGRIND_TESTS))
	@set -e; for t in $(VALGRIND_TESTS); do echo "== valgrind $$t"; \
	  valgrind --quiet --error-exitcode=1 --errors-for-leak-kinds=none ./$(BUILD)/$$t > /dev/null; done

test: $(addprefix $(BUILD)/,$(TESTS))
	@set -e; for t in $(TESTS); do echo "== $$t"; ./$(BUILD)/$$t; done

doc:
	cd doc/doxygen && doxygen Doxyfile

clean:
	rm -rf $(BUILD) doc/doxygen/html
