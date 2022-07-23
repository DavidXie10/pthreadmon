
#ifndef POKEMON_H
#define POKEMON_H

#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "mapper.h"

#define POKEMONS 6  // Number of battle pokemons
#define BONUS 2.3   // Attack bonus constant

typedef struct timespec walltime_t;     // to measure execution time

// Typedef of a pokemon struct
typedef struct p{
    int active;                         // Flag that indicates wether or not a pokemon is in the battle
    int num_thread;                     // The number of thread of the pokemon
    int hit_points;                     // The remaining life
    int energy_accumulated;             // The energy accumulated to do a charged attack
    pokemon_info_t* pokedex;            // The info struct from the mapper file with some basic information
    move_info_t* fast_move;             // The move struct that points to it's fast move attack information
    move_info_t* charge_move;           // The move struct that points to it's charge move attack information
    struct p* next_oponent;             // Pointer to the next pokemon to enter the battle, NULL if it is the last one
    walltime_t* start;                  // To measure it's battle (execution) time
}pokemon_t;

// Typedef of everything the pokemon needs to know when entering the arena
typedef struct pokemon_data{
    pokemon_t* my_pokemon;              // Pointer to a pokemon_t structure
    pokemon_t* opponente_info;          // Pointer to it's opponent
    pthread_mutex_t* my_turn;           // It's ticket to enter the battle arena
} pokemon_data_t;

/**
 * @brief The parallel function that all the 6 pokemon threads will be executing, but only 2 will be attacking each other at the same time, while the other 2 pokemons from each team will be waiting their turn.
 * @param arg: a pointer to a pokemon_data_t structure with all the information a pokemon needs to know before entering the battle
 * @return NULL
 */
void* attack(void* arg);

/**
 * @brief Calculates the damage inflicted by a pokemon according to the effectiveness of the attack
 * @param poke_attack: the fast move or charge move information
 * @param opponent_type: the opponent type of the pokemon to determine if it is effective, resistant o inmune to the attack
 * @return the value of the attack according to the formula provided
 */
int calculate_damage(move_info_t* poke_attack, pokemon_t* opponent_type);

/**
 * @brief An amount of time of pause in the program, so that it can be seen what's going on
 * @param how many miliseconds wants to pause the program
 */
void fixed_sleep(useconds_t duration);

/**
 * @brief Create and initialize an structure of type pokemon_t with it's attributes
 * @param num_thread: it's number of thread
 * @param id: it's identifier in the mapper file
 * @param next_oponent: a pointer to it's next opponent
 * @return a pokemon_t structure created in dynamic memory
 */
pokemon_t* pokemon_create(const int num_thread, const int id, pokemon_t *next_oponent);

/**
 * @brief Free the pokemon and destroy it, take back it's memory
 * @param pokemon: a pointer to a pokemon_t structure to be destroyed
 */
void pokemon_destroy(pokemon_t* pokemon);

/**
 * @brief Starts the execution time of a thread
 * @param start: a pointer to a walltime_t structure
 */
void walltime_start(walltime_t* start);

/**
 * @brief Measure how much time has passed since it has been called from walltime_start
 * @param start: a pointer to a walltime_t structure
 * @return the amount in seconds passed, the execution time battle of a thread
 */
double walltime_elapsed(const walltime_t* start);

#endif

