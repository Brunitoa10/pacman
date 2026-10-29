#ifndef PACMAN_CORE_PACMAN_H
#define PACMAN_CORE_PACMAN_H

#include <stdbool.h>

#include "core/entity.h"
#include "core/maze.h"

typedef struct {
    Entity body;
    Direction wanted; /* giro "en buffer": se aplica apenas sea posible */
} Pacman;

void pacman_spawn(Pacman *pacman, Vec2 at);
void pacman_steer(Pacman *pacman, Direction dir);

/* Devuelve true si Pac-Man entró a una casilla nueva en este tick. */
bool pacman_update(Pacman *pacman, const Maze *maze);

#endif /* PACMAN_CORE_PACMAN_H */
