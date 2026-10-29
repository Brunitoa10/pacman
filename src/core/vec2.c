#include "core/vec2.h"

Vec2 vec2_make(int row, int col)
{
    Vec2 v = { row, col };
    return v;
}

Vec2 vec2_add(Vec2 a, Vec2 b)
{
    return vec2_make(a.row + b.row, a.col + b.col);
}

Vec2 vec2_sub(Vec2 a, Vec2 b)
{
    return vec2_make(a.row - b.row, a.col - b.col);
}

Vec2 vec2_scale(Vec2 v, int factor)
{
    return vec2_make(v.row * factor, v.col * factor);
}

bool vec2_equals(Vec2 a, Vec2 b)
{
    return a.row == b.row && a.col == b.col;
}

int vec2_distance_sq(Vec2 a, Vec2 b)
{
    const int dr = a.row - b.row;
    const int dc = a.col - b.col;
    return dr * dr + dc * dc;
}
