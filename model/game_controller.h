#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "battle_arena.h"

// [0, 1, 2, 3, 4, 5, 6]
// 0, 1, 2: Team1
// 3, 4, 5: Team2
// 6: tiempo total
// Executions times of every pokemon and the total simulation time battle
double* execution_times;

// Array with the species names of each pokemon selected
char** pokemons_names;

// A pointer to a battle arena
battle_arena_t* battle_arena;

// Mutex for sincronization of the draw function with the creation and destruction of the battle arena
pthread_mutex_t battle_arena_mutex;

/**
 * @brief Its the "main" of the logic part. Creates the battle_arena and the threads of
 * each pokemon. At the end frees all the memory allocated in it and calls the destroys
 * of battle_arena. It also saves pokemon's times in execution_times array.
 * @param Receives arrays of ints that indicates all the ids of the pokemons to battle.
 * @return 0 if all executed well, otherwise its an error.
 */
int start_battle(int* id);

/**
 * @brief Prints in the terminal the time each pokemon was fighting.
 * @param A pointer to pokemon_data_list
 */
void show_results(pokemon_data_t* pokemon_data_list);
#endif

