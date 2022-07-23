#ifndef BATTLE_ARENA_H
#define BATTLE_ARENA_H

#include <stdio.h>
#include <stdlib.h>
#include "pokemon.h"

// Typedef struct of the battle arena
typedef struct poke_battle_arena{
    pokemon_t** team1;                      // Pointer to pointers of type pokemon_t that represents team1
    pokemon_t** team2;                      // Pointer to pointers of type pokemon_t that represents team2
    int team1_survivors;                    // Amount of team1 pokemons left
    int team2_survivors;                    // Amount of team2 pokemons left
    int num_threads;                        // Total threads in the program
    int charged_activated;                  // Flag that indicates if a pokemon is doing a charge attack
    walltime_t simulation_time;             // Measure the simulation time battle
    pthread_mutex_t* pokemons_mutex_array;  // pokemons_mutex for each thread to enter the arena, 0 - 2: team 1, 3 - 6: team 2
    pthread_mutex_t hit_points_mutex;       // For reducing the life of the opponents pokemons one at a time
    pthread_mutex_t charge_activate_mutex;  // Only one thread can perform a charge attack
    pthread_barrier_t barrier;              // Waits for all pokemons to be created and to enter the arena
    sem_t charged_move_sem;                 // If a pokemon activates it's charged moved then wait
} battle_arena_t;

/**
 * @brief Battle_arena_t creator. Initializes team1, team2 , team1_survivors, team2_survivors, num_threads, charged_activated,
 * simulation_time, pokemons_mutex_array, hit_points_mutex, charge_activate_mutex, barrier and charved_move_sem
 * @return A pointer to battle_arena_t
 */
battle_arena_t* battle_arena_create();

/**
 * @brief Battle_arena_t destroyer. Liberates memory of: pokemons_mutex_array[], calls pokemon_destroy for team 1 and team 2,
 * destroys mutex of charge_activate_mutex, hit_points_mute, destroys barrier and charged_move_sem.
 * Frees pointers to pokemons_mutex_array, team1 and team2. Lastly frees battle_arena.
 * @param battle_arena
 */
void battle_arena_destroy(battle_arena_t* battle_arena);

/**
 * @brief Returns the active pokemon in the array of team1.
 * @param Receives a pointer of batle_arena_t
 * @return The int (0,1,2) to tell which pokemon is active. In case none are active returns -1.
 */
int get_team1_active_pokemon(battle_arena_t* battle_arena);

/**
 * @brief Returns the active pokemon in the array of team2.
 * @param Receives a pointer of batle_arena_t
 * @return The int (0,1,2) to tell which pokemon is active. In case none are active returns -1.
 */
int get_team2_active_pokemon(battle_arena_t* battle_arena);

#endif
