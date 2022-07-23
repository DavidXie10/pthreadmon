
#include "../model/pokemon.h"
#include "../model/game_controller.h"
#include <semaphore.h>
#include <pthread.h>


void walltime_start(walltime_t* start){
    clock_gettime(CLOCK_MONOTONIC, start);
}

double walltime_elapsed(const walltime_t* start){
    walltime_t finish;
    clock_gettime(CLOCK_MONOTONIC, &finish);

    double elapsed = (finish.tv_sec - start->tv_sec);
    elapsed += (finish.tv_nsec - start->tv_nsec) / 1000000000.0;

    return elapsed;
}

void fixed_sleep(useconds_t duration){
    usleep( 1000 * duration );
}

//floor(power * effectiveness * bonus) + 1
int calculate_damage(move_info_t* poke_attack, pokemon_t* opponent_type){

    int damage = 0;
    double effectiveness = 1.0;
    if (weaknesses_matrix[opponent_type->pokedex->typeId][poke_attack->typeId])
        effectiveness = 1.6;
    else if (resistances_matrix[opponent_type->pokedex->typeId][poke_attack->typeId])
        effectiveness = 0.625;
    else if (immunities_matrix[opponent_type->pokedex->typeId][poke_attack->typeId])
        effectiveness = 0.390625;

    damage = floor((double)poke_attack->power * effectiveness * BONUS) + 1;
    return damage;
}

pokemon_t* pokemon_create(const int num_thread, const int id, pokemon_t *next_oponent){
    pokemon_t* pokemon = (pokemon_t*)malloc(sizeof (pokemon_t));
    if(!pokemon)
         return fprintf(stderr, "Error: could not allocate memory for pokemon\n"), NULL;

    pokemon->num_thread = num_thread;
    pokemon->hit_points = 1500;
    pokemon->energy_accumulated = 0;
    pokemon->active = 0;

    pokemon_info_t* poke_info = (pokemon_info_t*)malloc(sizeof (pokemon_info_t));
    if(!poke_info)
         return free(pokemon), fprintf(stderr, "Error: could not allocate memory for pokemon_info\n"), NULL;

    poke_info->id = id;
    poke_info->speciesName = get_pokemon_species_name(id);
    poke_info->fastMoveId = get_pokemon_fast_move_id(id);
    poke_info->chargedMoveId = get_pokemon_charged_move_id(id);
    poke_info->typeId = get_pokemon_type_id(id);

    move_info_t* fast_move = (move_info_t*)malloc(sizeof (move_info_t));
    if(!fast_move)
         return free(pokemon), free(poke_info), fprintf(stderr, "Error: could not allocate memory for fast_move\n"), NULL;

    int move_id = poke_info->fastMoveId;
    fast_move->id = move_id;
    fast_move->cooldown = get_move_cooldown(move_id);
    fast_move->moveName = get_move_name(move_id);
    fast_move->energy = get_move_energy(move_id);
    fast_move->energyGain = get_move_energy_gain(move_id);
    fast_move->power = get_move_power(move_id);
    fast_move->typeId = get_move_type_id(move_id);

    move_info_t* charged_moved = (move_info_t*)malloc(sizeof (move_info_t));
    if(!charged_moved)
         return free(pokemon), free(poke_info), free(fast_move), fprintf(stderr, "Error: could not allocate memory for charged_moved\n"), NULL;

    move_id = poke_info->chargedMoveId;
    charged_moved->id = move_id;
    charged_moved->cooldown = get_move_cooldown(move_id);
    charged_moved->moveName = get_move_name(move_id);
    charged_moved->energy = get_move_energy(move_id);
    charged_moved->energyGain = get_move_energy_gain(move_id);
    charged_moved->power = get_move_power(move_id);
    charged_moved->typeId = get_move_type_id(move_id);

    pokemon->start = (walltime_t*)malloc(sizeof (walltime_t));
    if(!pokemon->start)
         return free(pokemon), free(poke_info), free(fast_move), free(charged_moved), fprintf(stderr, "Error: could not allocate memory for walltime_t\n"), NULL;

    pokemon->next_oponent = next_oponent;
    pokemon->pokedex = poke_info;
    pokemon->fast_move = fast_move;
    pokemon->charge_move = charged_moved;

    return pokemon;
}

void pokemon_destroy(pokemon_t* pokemon){
    free(pokemon->pokedex);
    free(pokemon->fast_move);
    free(pokemon->charge_move);
    free(pokemon->start);
    free(pokemon);
}

void* attack(void* arg){
    pokemon_data_t* data = (pokemon_data_t*)arg;
    pokemon_t* my_pokemon = data->my_pokemon;
    pokemon_info_t* pokedex = my_pokemon->pokedex;
    move_info_t* fast_move = my_pokemon->fast_move;
    move_info_t* charge_move = my_pokemon->charge_move;
    pokemon_t* oponente_info = data->opponente_info;
    int num_thread = my_pokemon->num_thread;
    int energy = charge_move->energy;
    int energy_gain = fast_move->energyGain;

    int damage = 0;

    // Waits for all pokemons to enter the arena
    printf("%s has arrived at the arena!\n", pokedex->speciesName);
    pthread_barrier_wait(&battle_arena->barrier);

    pthread_mutex_lock(data->my_turn);
    while(oponente_info && oponente_info->hit_points <= 0)
        oponente_info = oponente_info->next_oponent;

    my_pokemon->active = 1;
    walltime_start(my_pokemon->start);

    while(oponente_info && my_pokemon->hit_points > 0){
        // If a pokemon activates it's charged moved then wait
        if(battle_arena->charged_activated)
            sem_wait(&battle_arena->charged_move_sem);

        // If the other pokemon kills me, then finish
        if(my_pokemon->hit_points <= 0)
            continue;

        if(my_pokemon->energy_accumulated < energy){
            damage = calculate_damage(fast_move, oponente_info);
            pthread_mutex_lock(&battle_arena->hit_points_mutex);
            fixed_sleep((useconds_t)500);
            printf("\n\n%s is attacking %d damage to %s\n", pokedex->speciesName, damage, oponente_info->pokedex->speciesName);
            oponente_info->hit_points -= damage;
            printf("Remaining life of %s = %d\n", oponente_info->pokedex->speciesName, oponente_info->hit_points);
            pthread_mutex_unlock(&battle_arena->hit_points_mutex);
            my_pokemon->energy_accumulated += energy_gain;
            fixed_sleep(fast_move->cooldown);
        }
        else{
            pthread_mutex_lock(&battle_arena->charge_activate_mutex);
            if(my_pokemon->hit_points){
                battle_arena->charged_activated = 1;
                damage = calculate_damage(charge_move, oponente_info);
                pthread_mutex_lock(&battle_arena->hit_points_mutex);
                printf("\n\n\t\t%s MADE CHARGED ATTACK %d damage to %s\n", pokedex->speciesName, damage, oponente_info->pokedex->speciesName);
                oponente_info->hit_points -= damage;
                printf("\t\tRemaining life of %s = %d\n", oponente_info->pokedex->speciesName, oponente_info->hit_points);
                my_pokemon->energy_accumulated -= energy;
                pthread_mutex_unlock(&battle_arena->hit_points_mutex);
                battle_arena->charged_activated = 0;
                sem_post(&battle_arena->charged_move_sem);
            }
            fixed_sleep((useconds_t)2000);   // animation
            pthread_mutex_unlock(&battle_arena->charge_activate_mutex);
            fixed_sleep(charge_move->cooldown);
        }

        if(oponente_info->hit_points <= 0)
            oponente_info = oponente_info->next_oponent;

    }
    my_pokemon->active = 0;

    // If I am from team 1
    if( (num_thread == 0 || num_thread == 1 || num_thread == 2) && my_pokemon->hit_points > 0)
        ++battle_arena->team1_survivors;
    else if( (num_thread == 3 || num_thread == 4 || num_thread == 5) && my_pokemon->hit_points > 0)
        ++battle_arena->team2_survivors;
    pthread_mutex_unlock(&battle_arena->pokemons_mutex_array[(num_thread + 1) % battle_arena->num_threads]);
    execution_times[num_thread] = walltime_elapsed(my_pokemon->start);

    return NULL;
}
