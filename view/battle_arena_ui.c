#include <gtk/gtk.h>
#include "../model/battle_arena.h"
#include "../model/game_controller.h"
#include "../model/mapper.h"

GtkWidget *window;
GtkWidget *grid;

// Pokemon selection for player 1
GtkComboBox * combo_box_player_1[3];

// Pokemon selection for player 2
GtkComboBox * combo_box_player_2[3];

// 0: HP1: label
// 1: Sprite: image
// 2: E1: label
GtkWidget *labels_pokemon_player_1[3];
GtkWidget *labels_pokemon_player_2[3];

// Charge attack label
GtkWidget *attack_info;

GtkWidget *button_start;

// Player 1 and 2 labels
GtkWidget *player1_label;
GtkWidget *player2_label;

static void my_callback(GObject *source_object, GAsyncResult *res, gpointer user_data){
    (void)source_object, (void)res, (void)user_data;
   /* Do nothing */
}

/**
 * @brief Checks if two pokemons in the same player are repeated by comparing ids.
 * @param Array with all the ids of the six pokemons
 * @return 1 if there is no repeated or 0 if there is repeated
 */
int is_valid(int* id){
    int next = 1;
    int index = 0;
    for(index = 0; index < 2; ++index){
        for(next = index + 1; next < 3; ++next ){
            if(id[index] == id[next] || id[index] == -1 || id[next] == -1)
                return 0;
        }
    }
    for(index = 3; index < POKEMONS - 1; ++index){
        for(next = index + 1; next < POKEMONS; ++next ){
            if(id[index] == id[next] || id[index] == -1 || id[next] == -1)
                return 0;
        }
    }
    return 1;
}

/**
 * @brief Initializes the six combo boxes in the UI for the player 1 and player 2 to choose the pokemon.
 */
void set_combo_boxes(){

    player1_label = gtk_label_new("Player 1");
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(player1_label), 1, 1, 1, 1);
    player2_label = gtk_label_new("Player 2");
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(player2_label), 5, 1, 1, 1);

    GtkWidget *combo_box;

    for (int i = 0; i < 3; ++i){
        combo_box = gtk_combo_box_text_new();
        for (int i = 0; i < NUM_POKEMON; ++i)
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box), get_pokemon_species_name(i));

        combo_box_player_1[i] = GTK_COMBO_BOX(combo_box);
        gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(combo_box_player_1[i]), 1, i+2, 1, 1);

        combo_box = gtk_combo_box_text_new();
        for (int i = 0; i < NUM_POKEMON; ++i)
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box), get_pokemon_species_name(i));

        combo_box_player_2[i] = GTK_COMBO_BOX(combo_box);
        gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(combo_box_player_2[i]), 5, i+2, 1, 1);
    }
}

/**
 * @brief Initializes the two main sections were the teams are going to be.
 * Team 1 with hit points, below the sprite, below the energy, that is repeated for team two.
 * Also initializes the label were the Charge Attack is going to be shown.
 */
void set_pokemon_spaces(){
    // Team1 [0, 1, 2, 3] ->
    labels_pokemon_player_1[0] = gtk_label_new("HP1");
    labels_pokemon_player_1[1] = gtk_image_new();
    labels_pokemon_player_1[2] = gtk_label_new("E1");

    labels_pokemon_player_2[0] = gtk_label_new("HP2");
    labels_pokemon_player_2[1] = gtk_image_new();
    labels_pokemon_player_2[2] = gtk_label_new("E2");

    for (int i = 0; i < 3; ++i){
        gtk_grid_attach(GTK_GRID(grid), labels_pokemon_player_1[i], 2, i+2, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), labels_pokemon_player_2[i], 4, i+2, 1, 1);
    }

    attack_info = gtk_label_new("Fast attack/Charge attack");
    gtk_grid_attach(GTK_GRID(grid), attack_info, 3, 2, 1, 3);

}

/**
 * @brief Inserts the values of the combo_boxes of each player into an array of ids.
 * If there is no repeated pokemon in a player, sets off the battle and shows results at end.
 * Otherwise prints error message.
 * @param Gtask* task
 * @param gpointer source_object
 * @param gpointer task_data
 * @param GCancellable *cancellable
 */
static void start_async(GTask *task, gpointer source_object, gpointer task_data, GCancellable *cancellable){
    (void)task, (void)source_object, (void)task_data, (void)cancellable;

    int* id = (int*)calloc(6, sizeof(int));
    if(!id)
        return (void)fprintf(stderr, "Error: could not allocate memory for id\n");

    execution_times = (double*)calloc(7, sizeof (double));
    if(!execution_times)
        return (void)fprintf(stderr, "Error: could not allocate memory for execution times\n"), free(id);

    pokemons_names = (char**)malloc(sizeof(char*)*POKEMONS);
    if(!pokemons_names)
        return (void)fprintf(stderr, "Error: could not allocate memory for pokemons_names\n"), free(id), free(execution_times);

    for(int poke = 0; poke < POKEMONS; ++poke){
        pokemons_names[poke] = (char*)calloc(20, sizeof(char));
        if(!pokemons_names[poke])
            return (void)fprintf(stderr, "Error: could not allocate memory for pokemons names\n"), free(id), free(execution_times);
    }

    int index = 0;

    for (index = 0; index < 3; ++index)
        id[index] = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_box_player_1[index]));
    for (index = 3; index < POKEMONS; ++index)
        id[index] = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_box_player_2[index - 3]));

    if(!is_valid(id)){
        gtk_label_set_text(GTK_LABEL(attack_info), "Error: the same player cannot\n repeat pokemons\nor one combobox is empty");
    } else {
        // 1 if team 1 wins, 2 if team 2 wins
        int winner = start_battle(id);

        gtk_label_set_text(GTK_LABEL(labels_pokemon_player_1[0]), "HP1");
        gtk_image_clear(GTK_IMAGE(labels_pokemon_player_1[1]));
        gtk_label_set_text(GTK_LABEL(labels_pokemon_player_1[2]), "E1");
        gtk_label_set_text(GTK_LABEL(labels_pokemon_player_2[0]), "HP2");
        gtk_image_clear(GTK_IMAGE(labels_pokemon_player_2[1]));
        gtk_label_set_text(GTK_LABEL(labels_pokemon_player_2[2]), "E2");

        char buffer[325];
        sprintf(buffer, "----PLAYER %d IS THE WINNER! \\o/----\n\n"
                        "        Total battle time: %.2lf s\n\n"
                        "\t\t\t  Player 1\n"
                        "\t\t\t%s: %.2lf s\n"
                        "\t\t%s: %.2lf s\n"
                        "\t%s: %.2lf s\n\n"
                        "\t\t\t  Player 2\n"
                        "\t\t\t%s: %.2lf s\n"
                        "\t\t%s: %.2lf s\n"
                        "\t%s: %.2lf s\n"
                        , winner, execution_times[6], pokemons_names[0], execution_times[0]
                                                    , pokemons_names[1], execution_times[1]
                                                    , pokemons_names[2], execution_times[2]
                                                    , pokemons_names[3], execution_times[3]
                                                    , pokemons_names[4], execution_times[4]
                                                    , pokemons_names[5], execution_times[5]);
        gtk_label_set_text(GTK_LABEL(attack_info), buffer);
    }

    free(id);
    free(execution_times);
    for(int poke = 0; poke < POKEMONS; ++poke)
        free(pokemons_names[poke]);
    free(pokemons_names);
    gtk_widget_set_sensitive(GTK_WIDGET(button_start), TRUE);
}

static void start_clicked(){
    GCancellable *cancellable = g_cancellable_new();
    GTask *task = g_task_new(g_object_new(G_TYPE_OBJECT, NULL), cancellable, my_callback, NULL);
    g_task_run_in_thread(task, start_async);
    g_object_unref(task);
}

/**
 * @brief This method is continually checking the arrays of team1[active_pokemon_team_1] and for team 2 to update
 * the numbers shown in the window. Checks which pokemon is active to set its healbar, its sprite and its energy.
 * Also checks if a pokemon is doing an energy attack to prints the name of the attack on the attack_info widget.
 * If there is no battle_arena it doesn't do anything.
 * @param GtkWidget *widget
 * @param GdkEventExpose *event
 * @param gpointer data
 * @return Always returns TRUE until the window is closed.
 */
static gboolean draw_battle_arena(GtkWidget *widget, GdkEventExpose *event, gpointer data){
    (void)widget, (void)event, (void)data;

    pthread_mutex_lock(&battle_arena_mutex);
    if (battle_arena) {
        gtk_widget_set_sensitive(GTK_WIDGET(button_start), FALSE);
        int active_pokemon_team_1 = get_team1_active_pokemon(battle_arena);
        int active_pokemon_team_2 = get_team2_active_pokemon(battle_arena);

        if(active_pokemon_team_1 != -1 && active_pokemon_team_2 != -1){
            // HP1: active pokemon of team 1
            char life[10];
            sprintf(life, "%d", battle_arena->team1[active_pokemon_team_1]->hit_points);
            gtk_label_set_text(GTK_LABEL(labels_pokemon_player_1[0]), life);

            // Sprite de pokemon activo de team 1
            char buffer[30];
            sprintf(buffer, "%s/%s.png", "sprites", battle_arena->team1[active_pokemon_team_1]->pokedex->speciesName);
            gtk_image_set_from_file(GTK_IMAGE(labels_pokemon_player_1[1]), buffer);
            gtk_image_set_from_pixbuf(GTK_IMAGE(labels_pokemon_player_1[1]), gdk_pixbuf_scale_simple(gtk_image_get_pixbuf(GTK_IMAGE(labels_pokemon_player_1[1])), 100, 100, GDK_INTERP_NEAREST));

            // Energy de pokemon 1 activo
            char energy[20];
            sprintf(energy, "%d/%d", battle_arena->team1[active_pokemon_team_1]->energy_accumulated, battle_arena->team1[active_pokemon_team_1]->charge_move->energy);
            gtk_label_set_text(GTK_LABEL(labels_pokemon_player_1[2]), energy);

            // HP2: active pokemon of team 2
            sprintf(life, "%d", battle_arena->team2[active_pokemon_team_2]->hit_points);
            gtk_label_set_text(GTK_LABEL(labels_pokemon_player_2[0]), life);

            // Sprite de pokemon activo de team 2
            sprintf(buffer, "%s/%s.png", "sprites", battle_arena->team2[active_pokemon_team_2]->pokedex->speciesName);
            gtk_image_set_from_file(GTK_IMAGE(labels_pokemon_player_2[1]), buffer);
            gtk_image_set_from_pixbuf(GTK_IMAGE(labels_pokemon_player_2[1]), gdk_pixbuf_scale_simple(gtk_image_get_pixbuf(GTK_IMAGE(labels_pokemon_player_2[1])), 100, 100, GDK_INTERP_NEAREST));

            // Energy de pokemon 2 activo
            sprintf(energy, "%d/%d", battle_arena->team2[active_pokemon_team_2]->energy_accumulated, battle_arena->team2[active_pokemon_team_2]->charge_move->energy);
            gtk_label_set_text(GTK_LABEL(labels_pokemon_player_2[2]), energy);

            char buffer_2[30];
            if(battle_arena->team1[active_pokemon_team_1]->energy_accumulated >= battle_arena->team1[active_pokemon_team_1]->charge_move->energy){
                sprintf(buffer_2, "CHARGE ATTACK:\n%s used\n%s!-->", battle_arena->team1[active_pokemon_team_1]->pokedex->speciesName, battle_arena->team1[active_pokemon_team_1]->charge_move->moveName);
                gtk_label_set_text(GTK_LABEL(attack_info), buffer_2);
            }else if(battle_arena->team2[active_pokemon_team_2]->energy_accumulated >= battle_arena->team2[active_pokemon_team_2]->charge_move->energy){
                sprintf(buffer_2, "CHARGE ATTACK:\n<--%s used\n%s!", battle_arena->team2[active_pokemon_team_2]->pokedex->speciesName, battle_arena->team2[active_pokemon_team_2]->charge_move->moveName);
                gtk_label_set_text(GTK_LABEL(attack_info), buffer_2);
            }else{
                pthread_mutex_lock(&battle_arena->charge_activate_mutex);
                char buffer_3[200];
                sprintf(buffer_3, "FAST ATTACK:\n%s used %s! -->\n<-- %s used %s!", battle_arena->team1[active_pokemon_team_1]->pokedex->speciesName,
                                                               battle_arena->team1[active_pokemon_team_1]->fast_move->moveName,
                                                               battle_arena->team2[active_pokemon_team_2]->pokedex->speciesName,
                                                               battle_arena->team2[active_pokemon_team_2]->fast_move->moveName);
                gtk_label_set_text(GTK_LABEL(attack_info), buffer_3);
                pthread_mutex_unlock(&battle_arena->charge_activate_mutex);
            }
        }
    }
    pthread_mutex_unlock(&battle_arena_mutex);
    return TRUE;
}

/**
 * @brief It calls the initializer of mapper.h, of combo boxes and pokemon_spaces.
 * Also creates the window that is shown to the user. Lastly creates the button
 * to start the battle that is linked with start_clicked.
 * @param GtkApplication *app
 * @param gpointer user_data
 */
static void activate(GtkApplication *app, gpointer user_data){
    (void)user_data;
    /* create a new window, and set its title */
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "PTHREADMON: POKEMON BATTLE");
    gtk_window_set_default_size(GTK_WINDOW(window), 700, 500);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    /* construct the container and sets the rows / columns to be of the same size (homogeneeous) */
    grid = gtk_grid_new();
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);

    /* Put the grid in the window */
    gtk_container_add(GTK_CONTAINER(window), grid);

    /* create the button and set the event 'clicked' */
    button_start = gtk_button_new_with_label("START BATTLE!!!");
    g_signal_connect(button_start, "clicked", G_CALLBACK(start_clicked), NULL);

    /* place the button in the grid cell (2, 9), width=2 heigth=1 */
    gtk_grid_attach(GTK_GRID(grid), button_start, 2, 5, 3, 1);

    initialize_data();
    set_combo_boxes();
    set_pokemon_spaces();

    g_timeout_add(33, (GSourceFunc)draw_battle_arena, (gpointer)window);

    /* Show all the widgets in the window */
    gtk_widget_show_all(window);
}

int main(int argc, char* argv[]){
    pthread_mutex_init(&battle_arena_mutex, NULL);
    GtkApplication *app;
    int status;

    app = gtk_application_new("example.gtk.game_controller", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    pthread_mutex_destroy(&battle_arena_mutex);

    return status;
}
