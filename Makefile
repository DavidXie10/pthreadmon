#Adapted from <http://jeisson.ecci.ucr.ac.cr/progra2/2019b/ejemplos/>

CC=gcc
FLAGS=`pkg-config --cflags gtk+-3.0` -pthread -g
CFLAGS=$(FLAGS) 

all: bin/pthreadmon

bin/pthreadmon: controller/mapper.c controller/battle_arena.c controller/game_controller.c controller/pokemon.c 
	$(CC) $(CFLAGS) controller/mapper.c controller/battle_arena.c controller/game_controller.c controller/pokemon.c view/battle_arena_ui.c -o pthreadmon `pkg-config --libs gtk+-3.0` -lm

.PHONY: clean
clean:
	rm pthreadmon
