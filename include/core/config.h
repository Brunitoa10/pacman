#ifndef PACMAN_CORE_CONFIG_H
#define PACMAN_CORE_CONFIG_H

/*
 * Constantes de ajuste del juego. Centralizarlas evita "números mágicos"
 * dispersos por el código y permite balancear el juego desde un solo lugar.
 */

/* Dimensiones del laberinto (en casillas). */
#define MAZE_ROWS 31
#define MAZE_COLS 28
#define TUNNEL_LENGTH 6

/* Bucle principal: paso fijo de simulación. */
#define TICK_MS 25
#define TICKS_PER_SECOND (1000 / TICK_MS)
#define SECONDS_TO_TICKS(s) ((s) * TICKS_PER_SECOND)

/* Velocidades expresadas en "ticks por casilla" (menor = más rápido). */
#define PACMAN_MOVE_INTERVAL 4
#define GHOST_MOVE_INTERVAL 5
#define GHOST_FAST_MOVE_INTERVAL 4
#define GHOST_FRIGHTENED_INTERVAL 8
#define GHOST_EATEN_INTERVAL 2
#define GHOST_TUNNEL_INTERVAL 9

/* Vidas y puntaje. */
#define START_LIVES 3
#define EXTRA_LIFE_SCORE 10000
#define SCORE_PELLET 10
#define SCORE_POWER 50
#define SCORE_FIRST_GHOST 200

/* Duraciones (segundos). */
#define FRIGHTENED_SECONDS 7
#define FRIGHTENED_MIN_SECONDS 2
#define FRIGHTENED_FLASH_SECONDS 2
#define READY_SECONDS 2
#define DYING_SECONDS 2
#define LEVEL_CLEAR_SECONDS 2

/* Distancia (al cuadrado) a la que Clyde se "asusta" y vuelve a su esquina. */
#define CLYDE_SHY_DISTANCE_SQ 64

#endif /* PACMAN_CORE_CONFIG_H */
