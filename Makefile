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

TESTS := test_bitboard test_player test_moves test_matrix test_match

.PHONY: all test test-equiv players valgrind wasm clean doc

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

PLAYER44 := $(PLY)/player4.4.c $(PLY)/heuristic3.c $(COMMON) $(BITBOARD)

$(BUILD)/test_match: $(TST)/test_match.c $(PLAYER44) $(HDRS) | $(BUILD)
	$(CC) $(CFLAGS) -pthread $(filter %.c,$^) -o $@ $(LDLIBS)

$(BUILD)/equiv_threads: $(TST)/test_equiv.c $(PLAYER44) $(HDRS) | $(BUILD)
	$(CC) $(CFLAGS) -pthread $(filter %.c,$^) -o $@ $(LDLIBS)

# pthreads live in libc on macOS and recent glibc: renaming pthread_create
# turns any leftover thread use in the sequential build into a link error.
$(BUILD)/equiv_seq: $(TST)/test_equiv.c $(PLAYER44) $(HDRS) | $(BUILD)
	$(CC) $(CFLAGS) -DGOMOKU_NO_THREADS -Dpthread_create=gomoku_threads_forbidden $(filter %.c,$^) -o $@ $(LDLIBS)

# The WebAssembly build has no threads: both builds must pick the same moves.
test-equiv: $(BUILD)/equiv_threads $(BUILD)/equiv_seq
	./$(BUILD)/equiv_threads | grep '^MOVE' > $(BUILD)/equiv_threads.txt
	./$(BUILD)/equiv_seq | grep '^MOVE' > $(BUILD)/equiv_seq.txt
	diff $(BUILD)/equiv_threads.txt $(BUILD)/equiv_seq.txt
	@echo "threaded and sequential 4.4 agree"

test: $(addprefix $(BUILD)/,$(TESTS)) test-equiv
	@set -e; for t in $(TESTS); do echo "== $$t"; ./$(BUILD)/$$t; done

doc:
	cd doc/doxygen && doxygen Doxyfile

clean:
	rm -rf $(BUILD) doc/doxygen/html

EMCC     ?= emcc
WASM_OUT := web/src/wasm/generated
EMFLAGS  := -O3 -std=c99 -Wall -sMODULARIZE=1 -sEXPORT_ES6=1 \
            -sENVIRONMENT=web,worker,node -sALLOW_MEMORY_GROWTH=1 \
            -sSTACK_SIZE=1048576 -sFILESYSTEM=0

wasm: $(WASM_OUT)/referee.js $(WASM_OUT)/player41.js $(WASM_OUT)/player44.js

$(WASM_OUT)/referee.js: src/wasm/referee_api.c $(BITBOARD) $(HDRS)
	$(EMCC) $(EMFLAGS) $(filter %.c,$^) -o $@

$(WASM_OUT)/player41.js: src/wasm/player_api.c $(PLY)/player4.c $(PLY)/heuristic0.c $(COMMON) $(HDRS)
	$(EMCC) $(EMFLAGS) $(filter %.c,$^) -o $@

$(WASM_OUT)/player44.js: src/wasm/player_api.c $(PLAYER44) $(HDRS)
	$(EMCC) $(EMFLAGS) -DGOMOKU_NO_THREADS -DPLAYER_HAS_DEPTH $(filter %.c,$^) -o $@
