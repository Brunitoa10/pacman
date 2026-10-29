#ifndef PACMAN_CORE_MAZE_H
#define PACMAN_CORE_MAZE_H

#include <stdbool.h>

#include "core/config.h"
#include "core/direction.h"
#include "core/vec2.h"

#define MAZE_MAX_GHOST_SPAWNS 4

typedef enum {
    TILE_EMPTY,
    TILE_WALL,
    TILE_DOOR,
    TILE_PELLET,
    TILE_POWER
} Tile;

/*
 * El laberinto es puro dato: se carga desde un layout de texto, así que
 * agregar un mapa nuevo no requiere tocar código (principio Abierto/Cerrado).
 *
 * Símbolos del layout:
 *   '#' pared   '.' punto   'o' power pellet   '-' puerta de la casa
 *   'P' inicio de Pac-Man   '0'..'3' inicio de cada fantasma   ' ' vacío
 */
typedef struct {
    Tile tiles[MAZE_ROWS][MAZE_COLS];
    bool tunnel_rows[MAZE_ROWS];
    int pellets_left;
    Vec2 pacman_spawn;
    Vec2 ghost_spawns[MAZE_MAX_GHOST_SPAWNS];
    Vec2 house_exit;   /* casilla justo encima de la puerta */
    Vec2 house_center; /* casilla dentro de la casa */
} Maze;

bool maze_load(Maze *maze, const char *const layout[MAZE_ROWS]);
void maze_load_classic(Maze *maze);

Tile maze_tile_at(const Maze *maze, Vec2 pos);
bool maze_is_walkable(const Maze *maze, Vec2 pos, bool can_pass_door);
bool maze_is_tunnel(const Maze *maze, Vec2 pos);
Vec2 maze_neighbor(Vec2 pos, Direction dir);

/* Come lo que haya en la casilla. Devuelve TILE_PELLET, TILE_POWER o TILE_EMPTY. */
Tile maze_consume(Maze *maze, Vec2 pos);
int maze_pellets_left(const Maze *maze);

#endif /* PACMAN_CORE_MAZE_H */
