#ifndef PACMAN_CORE_VEC2_H
#define PACMAN_CORE_VEC2_H

#include <stdbool.h>

/* Posición o desplazamiento en la grilla del laberinto. */
typedef struct {
    int row;
    int col;
} Vec2;

Vec2 vec2_make(int row, int col);
Vec2 vec2_add(Vec2 a, Vec2 b);
Vec2 vec2_sub(Vec2 a, Vec2 b);
Vec2 vec2_scale(Vec2 v, int factor);
bool vec2_equals(Vec2 a, Vec2 b);
int vec2_distance_sq(Vec2 a, Vec2 b);

#endif /* PACMAN_CORE_VEC2_H */
