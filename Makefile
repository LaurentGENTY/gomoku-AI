####### Flags #########
#######################

CFLAGS=-Wall -std=c99
CC=gcc -O0 -g
C_SPECIAL=gcc -O3
full+=OFF

BUILD_TYPE=Debug
BUILD_DIR=build_$(BUILD_TYPE)
BUILD_TEST_DIR=$(BUILD_DIR)/test

SRC_SERVER=src/server/
PLAYS=src/players/
TESTS=src/test/
DOC=doc/doxygen

.PHONY: all build build_dir clean install doc

all: build scientific_test

build: build_dir server $(BUILD_DIR)/player4.so $(BUILD_DIR)/player1.so $(BUILD_DIR)/player4.2.so $(BUILD_DIR)/player4.3.so $(BUILD_DIR)/player4.4.so

scientific_test: build_dir server_test $(BUILD_DIR)/player_test.so

#### Object files #####
#######################

$(BUILD_DIR)/%.o: $(SRC_SERVER)%.c
	@$(CC) ${CFLAGS} -c $< -o $@

$(BUILD_DIR)/list.o : $(PLAYS)list.c
	@$(CC) ${CFLAGS} -c $< -o $@

$(BUILD_DIR)/heuristic.o : $(PLAYS)heuristic.c
	@$(CC) ${CFLAGS} -c $< -o $@

$(BUILD_DIR)/heuristic2.o : $(PLAYS)heuristic2.c
	@$(CC) ${CFLAGS} -c $< -o $@

$(BUILD_DIR)/matrix.o : $(PLAYS)matrix.c
	@$(CC) ${CFLAGS} -c $< -o $@

##### For tests ######

$(BUILD_TEST_DIR)/test_game.o : $(TESTS)test_game.c $(SRC_SERVER)game.h $(SRC_SERVER)board.h $(TESTS)test_utils.h
	@$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) $(CFLAGS) -o $(BUILD_TEST_DIR)/test_game $(BUILD_TEST_DIR)/test_game.o $(BUILD_DIR)/game.o -lm

$(BUILD_TEST_DIR)/test_bitboard.o : $(TESTS)test_bitboard.c $(SRC_SERVER)bitboard.h $(SRC_SERVER)board.h $(PLAYS)list.h $(TESTS)test_utils.h
	@$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) $(CFLAGS) -o $(BUILD_TEST_DIR)/test_bitboard $(BUILD_TEST_DIR)/test_bitboard.o $(BUILD_DIR)/bitboard.o $(BUILD_DIR)/list.o -lm

$(BUILD_TEST_DIR)/test_moves.o : $(TESTS)test_moves.c $(SRC_SERVER)moves.h $(SRC_SERVER)move.h $(SRC_SERVER)board.h $(SRC_SERVER)bitboard.h $(TESTS)test_utils.h $(PLAYS)list.h
	@$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) $(CFLAGS) -o $(BUILD_TEST_DIR)/test_moves $(BUILD_TEST_DIR)/test_moves.o $(BUILD_DIR)/moves.o $(BUILD_DIR)/bitboard.o $(BUILD_DIR)/list.o -lm

$(BUILD_TEST_DIR)/test_player.o : $(TESTS)test_player.c $(PLAYS)heuristic.h $(PLAYS)player.h
	@$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) $(CFLAGS) -o $(BUILD_TEST_DIR)/test_player $(BUILD_TEST_DIR)/test_player.o $(BUILD_DIR)/heuristic2.o \
					$(BUILD_DIR)/matrix.o $(BUILD_DIR)/list.o $(BUILD_DIR)/bitboard.o -lm

$(BUILD_TEST_DIR)/test_matrix.o : $(TESTS)test_matrix.c $(PLAYS)matrix.h $(PLAYS)list.h
	@$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) $(CFLAGS) -o $(BUILD_TEST_DIR)/test_matrix $(BUILD_TEST_DIR)/test_matrix.o $(BUILD_DIR)/matrix.o $(BUILD_DIR)/list.o


###### Binaries #######
#######################

$(BUILD_TEST_DIR)/test_game : build_dir $(BUILD_DIR)/game.o $(BUILD_TEST_DIR)/test_game.o val_test_game.txt
	@if [ "$(shell grep -c "All heap blocks were freed" build_Debug/test/val_test_game.txt)"  = "1" ];\
	 then echo "Test free valgrind : SUCCESS"; else echo "Test free valgrind : FAILURE"; fi
	@if [ "$(shell grep -c "ERROR SUMMARY: 0" build_Debug/test/val_test_game.txt)"  = "1" ];\
	 then echo "Test error valgrind : SUCCESS"; else echo "Test error valgrind : FAILURE"; fi
	@echo ""

$(BUILD_TEST_DIR)/test_bitboard : build_dir $(BUILD_DIR)/list.o $(BUILD_DIR)/bitboard.o $(BUILD_TEST_DIR)/test_bitboard.o val_test_bitboard.txt
	@if [ "$(shell grep -c "All heap blocks were freed" build_Debug/test/val_test_bitboard.txt)"  = "1" ];\
	 then echo "Test free valgrind : SUCCESS"; else echo "Test free valgrind : FAILURE"; fi
	@if [ "$(shell grep -c "ERROR SUMMARY: 0" build_Debug/test/val_test_bitboard.txt)" = "1" ];\
	 then echo "Test error valgrind : SUCCESS"; else echo "Test error valgrind : FAILURE"; fi
	@echo ""

$(BUILD_TEST_DIR)/test_moves : build_dir $(BUILD_DIR)/moves.o $(BUILD_TEST_DIR)/test_moves.o val_test_moves.txt
	@if [ "$(shell grep -c "All heap blocks were freed" build_Debug/test/val_test_moves.txt)"  = "1" ];\
	 then echo "Test free valgrind : SUCCESS"; else echo "Test free valgrind : FAILURE"; fi
	@if [ "$(shell grep -c "ERROR SUMMARY: 0" build_Debug/test/val_test_moves.txt)" = "1" ];\
	 then echo "Test error valgrind : SUCCESS"; else echo "Test error valgrind : FAILURE"; fi
	@echo ""

$(BUILD_TEST_DIR)/test_player : build_dir  $(BUILD_DIR)/heuristic2.o $(BUILD_DIR)/matrix.o $(BUILD_DIR)/list.o $(BUILD_DIR)/bitboard.o \
		$(BUILD_TEST_DIR)/test_player.o val_test_player.txt
	@if [ "$(shell grep -c "All heap blocks were freed" build_Debug/test/val_test_player.txt)"  = "1" ];\
	 then echo "Test free valgrind : SUCCESS"; else echo "Test free valgrind : FAILURE"; fi
	@if [ "$(shell grep -c "ERROR SUMMARY: 0" build_Debug/test/val_test_player.txt)" = "1" ];\
	 then echo "Test error valgrind : SUCCESS"; else echo "Test error valgrind : FAILURE"; fi
	@echo ""

$(BUILD_TEST_DIR)/test_matrix : build_dir  $(BUILD_DIR)/matrix.o $(BUILD_DIR)/list.o $(BUILD_TEST_DIR)/test_matrix.o val_test_matrix.txt
	@if [ "$(shell grep -c "All heap blocks were freed" build_Debug/test/val_test_matrix.txt)"  = "1" ];\
	 then echo "Test free valgrind : SUCCESS"; else echo "Test free valgrind : FAILURE"; fi
	@if [ "$(shell grep -c "ERROR SUMMARY: 0" build_Debug/test/val_test_matrix.txt)" = "1" ];\
	 then echo "Test error valgrind : SUCCESS"; else echo "Test error valgrind : FAILURE"; fi
	@echo ""

# txt files for tests #
#######################

val_test_game.txt: $(BUILD_TEST_DIR)/test_game.o
	@valgrind --log-file='build_Debug/test/val_test_game.txt' $(BUILD_TEST_DIR)/test_game $(full)

val_test_bitboard.txt: $(BUILD_TEST_DIR)/test_bitboard.o
	@valgrind --log-file='build_Debug/test/val_test_bitboard.txt' $(BUILD_TEST_DIR)/test_bitboard $(full)

val_test_moves.txt: $(BUILD_TEST_DIR)/test_moves.o
	@valgrind --log-file='build_Debug/test/val_test_moves.txt' $(BUILD_TEST_DIR)/test_moves $(full)

val_test_player.txt: $(BUILD_TEST_DIR)/test_player.o
	@valgrind --log-file='build_Debug/test/val_test_player.txt' $(BUILD_TEST_DIR)/test_player

val_test_matrix.txt: $(BUILD_TEST_DIR)/test_matrix.o
	@valgrind --log-file='build_Debug/test/val_test_matrix.txt' $(BUILD_TEST_DIR)/test_matrix

##### PLAYERS #####
## Dynamic Libraries ##
#######################

$(BUILD_DIR)/%.so :
	$(CC) $(CFLAGS) -o $@ $^ -fPIC -shared -lpthread -lm

######## rules ########
#######################

server_test: $(BUILD_DIR)/utils.o $(BUILD_DIR)/bitboard.o $(BUILD_DIR)/server_test.o $(BUILD_DIR)/moves.o $(BUILD_DIR)/players.o $(BUILD_DIR)/list.o $(BUILD_DIR)/evaluate_opening.o
	$(CC) ${CFLAGS} -o $(BUILD_DIR)/$@ $^ -ldl -lm

server: $(BUILD_DIR)/utils.o $(BUILD_DIR)/bitboard.o $(BUILD_DIR)/server.o $(BUILD_DIR)/moves.o $(BUILD_DIR)/players.o $(BUILD_DIR)/list.o
	$(CC) ${CFLAGS} -o $(BUILD_DIR)/$@ $^ -ldl -lm

doc:
	cd $(DOC) && doxygen Doxyfile

build_dir:
	@if [ ! -d $(BUILD_DIR) ]; then mkdir $(BUILD_DIR); fi
	@if [ ! -d $(BUILD_TEST_DIR) ]; then mkdir $(BUILD_TEST_DIR); fi

test : $(BUILD_TEST_DIR)/test_game $(BUILD_TEST_DIR)/test_bitboard $(BUILD_TEST_DIR)/test_moves $(BUILD_TEST_DIR)/test_player $(BUILD_TEST_DIR)/test_matrix

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(DOC)/html
	rm -rf $(PLAYS)*.o $(PLAYS)*~ 
	rm -f *.o server *.so test_*
	rm -f install/*

install: build
	if [ ! -d "install" ]; then mkdir install; fi
	cp $(BUILD_DIR)/server install/server
	cp $(BUILD_DIR)/player1.so install/player9999.so
	cp $(BUILD_DIR)/player4.so install/player4.so
	cp $(BUILD_DIR)/player4.2.so install/player4.2.so
	cp $(BUILD_DIR)/player4.3.so install/player4.3.so
	cp $(BUILD_DIR)/player4.4.so install/player4.4.so

#### Dependencies #####
#######################

### Regenerate with ###
## gcc -MM <from $(PLAYS)*.c> ##
# and copy paste here #

server_test.o: server_test.c move.h game.h board.h utils.h moves.h players.h bitboard.h
bitboard.o: $(SRC_SERVER)bitboard.c $(SRC_SERVER)bitboard.h $(SRC_SERVER)move.h \
 $(SRC_SERVER)board.h $(SRC_SERVER)../players/list.h \
 $(SRC_SERVER)../players/../server/move.h
game.o: $(SRC_SERVER)game.c $(SRC_SERVER)game.h $(SRC_SERVER)move.h \
 $(SRC_SERVER)board.h
moves.o: $(SRC_SERVER)moves.c $(SRC_SERVER)move.h $(SRC_SERVER)moves.h \
 $(SRC_SERVER)board.h
players.o: $(SRC_SERVER)players.c $(SRC_SERVER)move.h $(SRC_SERVER)moves.h \
 $(SRC_SERVER)board.h $(SRC_SERVER)players.h
server.o: $(SRC_SERVER)server.c $(SRC_SERVER)move.h $(SRC_SERVER)game.h \
 $(SRC_SERVER)board.h $(SRC_SERVER)utils.h $(SRC_SERVER)moves.h \
 $(SRC_SERVER)players.h $(SRC_SERVER)bitboard.h
server_test.o: $(SRC_SERVER)server_test.c $(SRC_SERVER)move.h \
 $(SRC_SERVER)game.h $(SRC_SERVER)board.h $(SRC_SERVER)utils.h \
 $(SRC_SERVER)moves.h $(SRC_SERVER)players.h $(SRC_SERVER)bitboard.h
utils.o: $(SRC_SERVER)utils.c $(SRC_SERVER)utils.h

##### players #####

heuristic0.o: heuristic0.c heuristic0.h matrix.h list.h ../server/move.h
heuristic2.o: heuristic2.c matrix.h list.h ../server/move.h heuristic2.h \
 ../server/board.h ../server/move.h
heuristic3.o: heuristic3.c matrix.h list.h ../server/move.h heuristic3.h \
 ../server/board.h ../server/move.h
heuristic.o: heuristic.c matrix.h list.h ../server/move.h heuristic.h \
 ../server/board.h ../server/move.h
list.o: list.c list.h ../server/move.h
matrix.o: matrix.c matrix.h list.h ../server/move.h
montecarlo.o: montecarlo.c montecarlo.h ../server/move.h matrix.h list.h \
 ../server/board.h ../server/move.h
player1.o: player1.c player.h ../server/move.h
player2.o: player2.c player.h ../server/move.h
player3.o: player3.c ../server/move.h player.h matrix.h list.h \
 heuristic0.h
player4.2.o: player4.2.c player.h ../server/move.h matrix.h list.h \
 heuristic.h ../server/board.h ../server/move.h
player4.3.o: player4.3.c player.h ../server/move.h matrix.h list.h \
 heuristic2.h ../server/board.h ../server/move.h
player4.4.o: player4.4.c player.h ../server/move.h matrix.h list.h \
 heuristic3.h ../server/board.h ../server/move.h
player4.o: player4.c player.h ../server/move.h matrix.h list.h \
 heuristic0.h
player5.o: player5.c player.h ../server/move.h matrix.h list.h \
 montecarlo.h ../server/board.h ../server/move.h
player_test.o: $(PLAYS)player_test.c $(PLAYS)player.h  $(PLAYS)move.h

### dependancies for dynamic libraries

$(BUILD_DIR)/player1.so : $(PLAYS)player1.c $(PLAYS)player.h $(SRC_SERVER)move.h

$(BUILD_DIR)/player1.so : $(PLAYS)player1.c $(PLAYS)player.h $(SRC_SERVER)move.h

$(BUILD_DIR)/player2.so : $(PLAYS)player2.c $(PLAYS)player.h $(SRC_SERVER)move.h

$(BUILD_DIR)/player3.so : $(PLAYS)player3.c $(PLAYS)player.h $(SRC_SERVER)move.h $(PLAYS)matrix.c $(PLAYS)matrix.h $(PLAYS)list.c $(PLAYS)list.h $(PLAYS)heuristic0.c $(PLAYS)heuristic0.h

$(BUILD_DIR)/player4.so : $(PLAYS)player4.c $(PLAYS)player.h $(SRC_SERVER)move.h $(PLAYS)matrix.c $(PLAYS)matrix.h $(PLAYS)list.c $(PLAYS)list.h $(PLAYS)heuristic0.c $(PLAYS)heuristic0.h

$(BUILD_DIR)/player4.2.so : $(PLAYS)player4.2.c $(PLAYS)player.h $(SRC_SERVER)move.h $(PLAYS)matrix.c $(PLAYS)matrix.h $(PLAYS)list.c $(PLAYS)list.h $(PLAYS)heuristic.c $(PLAYS)heuristic.h $(SRC_SERVER)bitboard.c $(SRC_SERVER)bitboard.h

$(BUILD_DIR)/player4.3.so : $(PLAYS)player4.3.c $(PLAYS)player.h $(SRC_SERVER)move.h $(PLAYS)matrix.c $(PLAYS)matrix.h $(PLAYS)list.c $(PLAYS)list.h $(PLAYS)heuristic2.c $(PLAYS)heuristic2.h $(SRC_SERVER)bitboard.c $(SRC_SERVER)bitboard.h

$(BUILD_DIR)/player4.4.so : $(PLAYS)player4.4.c $(PLAYS)player.h $(SRC_SERVER)move.h $(PLAYS)matrix.c $(PLAYS)matrix.h $(PLAYS)list.c $(PLAYS)list.h $(PLAYS)heuristic3.c $(PLAYS)heuristic3.h $(SRC_SERVER)bitboard.c $(SRC_SERVER)bitboard.h

$(BUILD_DIR)/player5.so : $(PLAYS)player5.c $(PLAYS)player.h $(SRC_SERVER)move.h $(PLAYS)matrix.c $(PLAYS)matrix.h $(PLAYS)list.c $(PLAYS)list.h $(PLAYS)montecarlo.c $(PLAYS)montecarlo.h

$(BUILD_DIR)/player_test.so : $(PLAYS)player_test.c $(PLAYS)player.h  $(SRC_SERVER)move.h
