# CI0117 - Proyecto 1: Simulador de Batallas de Hilos [pthreadmon]

**Fecha de entrega:** Jueves 22 de Octubre 11:50pm.

## Equipo de desarrollo

* **Juan Ignacio Pacheco (B85841)**
* **David Xie Li (B88682)**

Este proyecto es parte del curso de CI-0117 Programación Paralela y Concurrente de la Universidad de Costa Rica. Realizado durante el segundo semestre del año 2020.

## Manual de usuario.

Primero que todo, en caso de no tener la biblioteca de gtk instalado, hacerlo con el siguiente comando:

        $ sudo apt-get install libgtk-3-dev

### Compilación

La manera de compilar la solución es a travéz del Makefile. Para compilar solo ocupa poner la siguiente instrucción desde la raíz del proyecto:

        $ make

### Ejecución

El nombre del ejecutable, una vez compilado, es **pthreadmon**, el cual para ejecutarse se invoca desde la raíz del proyecto y de la siguiente forma:

        $ ./pthreadmon

### Entradas esperadas

En cuanto a argumentos junto al ejecutable, no se espera ninguno. Las entradas se solicitan a la hora de ejecución en la interfaz gráfica y corresponderá a la selección de los 3 pokemones por jugador, dando un total de 6 pokemones. Estos se escogen a través de un combobox por pokemon y únicamente debe elegir el nombre de pokemon.

Cabe resaltar que el programa no se ejecutara y mostrará un mensaje de error si algún player tiene 2 o 3 pokemones iguales. Por lo que esperará hasta que ingrese los pokemones válidos para poder ejecutarse.

Una vez validados los pokemones, hay que presionar el botón de **START BATTLE!!!** para iniciar la simulación de batalla. Además, si se empieza una batalla, el botón de **START BATTLE!!!** se deshabilita.

Asimismo, una vez terminada la simulación, el usuario puede cambiar los pokemones a utilizar por cada jugador y empezar otra simulación.
En la interfaz, hay dos labels entre la imagen del pokemon activo, el de arriba indica los puntos de vida restantes del pokemon, mientras que el de abajo indica la cantidad de energía acumulada y la cantidad de energía a la que tiene que llegar para poder hacer ataque cargado, con el siguiente formato **energia acumulada/energia para realizar ataque cargado**.

### Interpretación de resultados
Cuando los 3 pokemones de un jugador quedan KO, la simulación termina. Luego se muestra en el centro de la interfaz, cuál de los jugadores ganó la batalla y las siguientes estadísticas:
- La duración total de la batalla.
- Tiempo que tardó cada Pokémon de cada jugador mientras estuvo activo.


## Detalles de la solución: Estructura del código fuente

El programa está estructurado en diferentes folders y archivos .c con sus respectivos archivos de encabezado .h basándose en el principio de separación de responsabilides. Siguiendo una architectura Modelo-Vista-Controlador (MVC) para organizar los archivos. Tenemos 3 folders principales que son el **model**, que tiene los headers o encabezados .h, el **controller**, que tiene los archivos .c, el **view** que tiene la función main y la interfaz gráfica del programa y **sprites** que posee las imagenes de los pokemones.

* Los archivos que pertenecen al folder model son: battle_arena.h, game_controller.h, mapper.h y pokemon.h.
* Los archivos que pertenecen al foler controller son: battle_arena.c, game_controller.c, mapper.c y pokemon.c.
* El archivo que pertenece al view es el battle_arena_ui.c

### Breve descripción de cada componente

- **Battle Arena:**

Es el campo de batalla, que se encarga de la creación del campo y su destrucción. Además conoce cuál es el pokemon activo de cada equipo. Posee un tipo definido llamado **battle_arena_t**, el cual tiene todas las variables compartidas por los threads y los métodos de sincronización.

- **Pokemon:**

Son los hilos que corren en paralelo y se encargan de atacarse, además tienen sus funciones para crearse y destruirse, cálculos de daño y le avisan al siguiente pokemon que se active cuando este muere. Contiene 2 tipos definidos, uno llamado **pokemon_t**, que contiene toda la información que necesita de sí mismo, tales como una bandera para saber si esta activo, sus puntos de vida, su número de thread, su energía acumulada, la información de su ataque rápido y cargado y un puntero al próximo oponente que sigue en caso de morir.

Y otro tipo definido llamado **pokemon_data_t**, que contiene toda la información que se le pasa a la funcion paralela, es decir un puntero al oponente contra el que está peleando, un puntero a su información de tipo **pokemon_t** y un puntero a un mutex que se desbloqueará cuando sea su turno.

- **Game Controller:**

Este módulo corresponde al controlador del juego y contiene la función que la interfaz llamará para correr, se encarga de la creación de los hilos, el battle_arena, la información privada de cada pokemon/thread, llama a la función paralela y se encarga también de la destrucción de todas las estructuras creadas en memoria dinámica para evitar fugas de memoria.

- **Battle Arena User Interface:**

Inicializa la interfaz gráfica y pone el programa a correr. Además dibuja lo que está ocurriendo durante el programa.

- **Mapper:**

Módulo suministrado por el profesor Jose Andrés Mena, que contiene la información personal de los 50 pokemones suministrados y los ataques.

### Interacción de los componentes

El battle_arena_ui empieza la ejecución del programa, ya que posee la función main, luego una vez elegidos correctamente los pokemones y presionado el botón de **START BATTLE!!!**, se llama a la función **start_battle(id)**, que se encuentra en el módulo de **game_controller**. Dentro de la función, se crea el **battle_arena**, los hilos de los pokemones y los **pokemon_t** correspondientes a cada pokemon seleccionado que se almacenarán en la estructura de tipo **pokemon_data_t** y luego se llama a la función **attack** del módulo de **pokemon** para que los hilos empiecen a atacarse. Asimismo, durante toda la ejecución del programa se van registrando los resultados que se mostrarán en las estadísticas al terminar la simulación.


## Métodos de sincronización

Los métodos de sincronización usados fueron los siguientes:

- **pthread_mutex_t* pokemons_mutex_array**

Este primer método consiste en un puntero a un arreglo de 6 mutexes, que controlan el ingreso correcto del pokemon a la batalla, donde los índices 0, 1 y 2 corresponden a los 3 pokemones del jugador 1, en ese orden y los índices 3, 4 y 5 a los 3 pokemones del jugador 2. Al principio, solo los mutexes 0 y 3 estarán desbloqueados, ya que esos son los 2 primeros pokemones que se enfrentarán y los otros mutexes estarán bloqueados.

- **pthread_mutex_t hit_points_mutex**

Se encarga de controlar la escritura y lectura de la vida de cada pokemon para que un pokemon ya muerto no vuelva a atacar.

- **pthread_mutex_t charge_activate_mutex**

Se encarga de controlar que solo un pokemon a la vez pueda hacer ataque cargado, en el caso de que ambos lo hayan alcanzado al mismo tiempo.

- **pthread_barrier_t barrier**

Espera a que los 6 pokemones lleguen hasta este punto para poder continuar con su ejecución.

- **sem_t charged_move_sem**

Controla que si un pokemon está haciendo ataque cargado, el otro no haga ataque rápido.

- **pthread_mutex_t battle_arena_mutex**

Este mutex permite controlar el acceso a las estructuras de datos de forma segura, de modo que no existan condiciones de carrera entre la interfaz y el método principal del programa.

### Problemas enfrentados

- Uno de los problemas que se nos presentó es que conforme se creaban los hilos (pokemones), estos empezaban a atacar sin esperar a que el oponente se creara. Por lo tanto el pokemon que llegara primero tenía ventaja sobre el oponente. Para resolver esto, se utilizó una barrera al inicio de la función paralela, para que ninguno empezara a atacar hasta que todos hayan llegado.

- Otro problema fue que la interfaz imprimía el mensaje de quién está haciendo ataque cargado en el centro, mientras que el otro pokemon que no debería estar haciendo nada, aumentaba su energía. Esto se arregló poniendo un mutex a la hora de cambiar el mensaje del ataque cargado a los 3 puntos suspensivos (estado default).


## Licencia

CC BY Proyecto 01 Programación Paralela y Concurrente - Simulador de Batallas de Hilos [pthreadmon]
