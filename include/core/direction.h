#ifndef PACMAN_CORE_DIRECTION_H
#define PACMAN_CORE_DIRECTION_H

#include "core/vec2.h"

/*
 * El orden UP, LEFT, DOWN, RIGHT es intencional: es la prioridad de
 * desempate que usan los fantasmas del arcade original.
 */
typedef enum {
    DIR_NONE = -1,
    DIR_UP = 0,
    DIR_LEFT,
    DIR_DOWN,
    DIR_RIGHT,
    DIR_COUNT
} Direction;

Vec2 direction_to_vec(Direction dir);
Direction direction_opposite(Direction dir);

#endif /* PACMAN_CORE_DIRECTION_H */
