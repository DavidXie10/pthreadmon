
#include "../model/battle_arena.h"

int get_team1_active_pokemon(battle_arena_t* battle_arena){
    for(int pokemon = 0; pokemon < 3; ++pokemon){
        if(battle_arena->team1[pokemon]->active)
            return pokemon;
    }
    return -1;
}

int get_team2_active_pokemon(battle_arena_t *battle_arena){
    for(int pokemon = 0; pokemon < 3; ++pokemon){
        if(battle_arena->team2[pokemon]->active)
            return pokemon;
    }
    return -1;
}

battle_arena_t* battle_arena_create(){
    battle_arena_t* battle_arena = (battle_arena_t*)calloc(1,sizeof(battle_arena_t));
    if(!battle_arena){
        fprintf(stderr, "Error: could not allocate memory for battle_arena\n");
        return NULL;
    }

    battle_arena->team1 = (pokemon_t**)calloc(3, sizeof(pokemon_t*));
    if(!battle_arena->team1)
        return free(battle_arena), fprintf(stderr, "Error: could not allocate memory for battle_arena->team1\n"), NULL;

    battle_arena->team2 = (pokemon_t**)calloc(3, sizeof(pokemon_t*));
    if(!battle_arena->team2)
        return free(battle_arena->team1), free(battle_arena), fprintf(stderr, "Error: could not allocate memory for battle_arena->team2\n"), NULL;

    battle_arena->pokemons_mutex_array = (pthread_mutex_t*)calloc(POKEMONS, sizeof(pthread_mutex_t));
    if(!battle_arena->pokemons_mutex_array){
        free(battle_arena->team2), free(battle_arena->team1);
        free(battle_arena);
        fprintf(stderr, "Error: could not allocate memory for battle_arena->pokemons_mutex_array\n");
        return NULL;
    }

    battle_arena->num_threads = POKEMONS;
    battle_arena->charged_activated = 0;
    battle_arena->team1_survivors = 0;
    battle_arena->team2_survivors = 0;

    pthread_mutex_init(&battle_arena->hit_points_mutex, NULL);
    pthread_barrier_init(&battle_arena->barrier, NULL, POKEMONS);
    for (int i = 0; i < 6; ++i)
        pthread_mutex_init(&battle_arena->pokemons_mutex_array[i], NULL);

    pthread_mutex_lock(&battle_arena->pokemons_mutex_array[1]);
    pthread_mutex_lock(&battle_arena->pokemons_mutex_array[2]);
    pthread_mutex_lock(&battle_arena->pokemons_mutex_array[4]);
    pthread_mutex_lock(&battle_arena->pokemons_mutex_array[5]);

    pthread_mutex_init(&battle_arena->charge_activate_mutex, NULL);
    sem_init(&battle_arena->charged_move_sem, 0, 0);

    return battle_arena;
}

// Destroy after pokemons destructions
// IT DESTROYS THE POKEMONS
void battle_arena_destroy(battle_arena_t* battle_arena){
    int index = 0;
    for (index = 0; index < POKEMONS; ++index)
        pthread_mutex_destroy(&battle_arena->pokemons_mutex_array[index]);

    for(index = 0; index < 3; ++index){
        pokemon_destroy(battle_arena->team1[index]);
        pokemon_destroy(battle_arena->team2[index]);
    }

    pthread_mutex_destroy(&battle_arena->charge_activate_mutex);
    pthread_mutex_destroy(&battle_arena->hit_points_mutex);
    pthread_barrier_destroy(&battle_arena->barrier);
    sem_destroy(&battle_arena->charged_move_sem);

    free(battle_arena->pokemons_mutex_array);
    free(battle_arena->team1);
    free(battle_arena->team2);
    free(battle_arena);
}
