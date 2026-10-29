#include "core/direction.h"

Vec2 direction_to_vec(Direction dir)
{
    switch (dir) {
    case DIR_UP:    return vec2_make(-1, 0);
    case DIR_LEFT:  return vec2_make(0, -1);
    case DIR_DOWN:  return vec2_make(1, 0);
    case DIR_RIGHT: return vec2_make(0, 1);
    default:        return vec2_make(0, 0);
    }
}

Direction direction_opposite(Direction dir)
{
    switch (dir) {
    case DIR_UP:    return DIR_DOWN;
    case DIR_LEFT:  return DIR_RIGHT;
    case DIR_DOWN:  return DIR_UP;
    case DIR_RIGHT: return DIR_LEFT;
    default:        return DIR_NONE;
    }
}
