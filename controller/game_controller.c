
#include "../model/game_controller.h"
#include <string.h>

// 0, const int id1, const int id2, const int id3, const int id4, const int id5
int start_battle(int* id){
    pthread_mutex_lock(&battle_arena_mutex);
    battle_arena = battle_arena_create();
    pthread_mutex_unlock(&battle_arena_mutex);

    if(!battle_arena)
        return fprintf(stderr, "Error: could not allocate memory for battle_arena\n"), 1;

    pokemon_data_t* pokemon_data_list = (pokemon_data_t*)calloc(POKEMONS, sizeof (pokemon_data_t));
    if(!pokemon_data_list)
        return free(battle_arena), fprintf(stderr, "Error: could not allocate memory for pokemon_data_list\n"), 2;

    pthread_t* pokemon_threads = (pthread_t*)calloc(POKEMONS, sizeof(pthread_t));
    if(!pokemon_threads)
        return free(pokemon_data_list), free(battle_arena), fprintf(stderr, "Error: could not allocate memory for pokemon_threads\n"), 3;
    int pokemon = 5;

    for(pokemon = 5; pokemon >= 0; --pokemon){
        pokemon_data_list[pokemon].my_pokemon = pokemon_create(pokemon, id[pokemon], (pokemon == 5 || pokemon == 2)?NULL:pokemon_data_list[pokemon+1].my_pokemon);
        strncpy(pokemons_names[pokemon], pokemon_data_list[pokemon].my_pokemon->pokedex->speciesName, 20);
        pokemon_data_list[pokemon].my_turn = &battle_arena->pokemons_mutex_array[pokemon];
    }

    walltime_start(&battle_arena->simulation_time);

    // Oponents info
    for(pokemon = 0; pokemon < 3; ++pokemon){
        pokemon_data_list[pokemon].opponente_info = pokemon_data_list[3].my_pokemon;
        battle_arena->team1[pokemon] = pokemon_data_list[pokemon].my_pokemon;
        pthread_create(&pokemon_threads[pokemon], NULL, attack, (void*)&pokemon_data_list[pokemon]);
    }

    for(pokemon = 3; pokemon < POKEMONS; ++pokemon){
        pokemon_data_list[pokemon].opponente_info = pokemon_data_list[0].my_pokemon;
        battle_arena->team2[pokemon - 3] = pokemon_data_list[pokemon].my_pokemon;
        pthread_create(&pokemon_threads[pokemon], NULL, attack, (void*)&pokemon_data_list[pokemon]);
    }

    for (pokemon = 0; pokemon < POKEMONS; ++pokemon)
        pthread_join(pokemon_threads[pokemon], NULL);

    double seconds_elapsed = walltime_elapsed(&battle_arena->simulation_time);
    execution_times[6] = seconds_elapsed;
    printf("Total simulation time: %.2lf s\n", seconds_elapsed);

    show_results(pokemon_data_list);

    int winner = 0;
    winner = battle_arena->team1_survivors > battle_arena->team2_survivors?1:2;

    pthread_mutex_lock(&battle_arena_mutex);
    battle_arena_destroy(battle_arena);
    battle_arena = NULL;
    free(pokemon_threads);
    free(pokemon_data_list);
    pthread_mutex_unlock(&battle_arena_mutex);

    return winner;
}

void show_results(pokemon_data_t* pokemon_data_list){
    printf("Active battle time:\n");
    for(int index = 0; index < POKEMONS; ++index){
        if(index < 3)
            printf("Team 1 %s duration: %.2lf s\n", pokemon_data_list[index].my_pokemon->pokedex->speciesName, execution_times[index]);
        else
            printf("Team 2 %s duration: %.2lf s\n", pokemon_data_list[index].my_pokemon->pokedex->speciesName, execution_times[index]);
    }
}
