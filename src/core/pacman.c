#include "core/pacman.h"

#include "core/config.h"

static bool can_move(const Pacman *pacman, const Maze *maze, Direction dir)
{
    if (dir == DIR_NONE) {
        return false;
    }
    return maze_is_walkable(maze, maze_neighbor(pacman->body.pos, dir), false);
}

void pacman_spawn(Pacman *pacman, Vec2 at)
{
    entity_place(&pacman->body, at, DIR_LEFT, PACMAN_MOVE_INTERVAL);
    pacman->wanted = DIR_LEFT;
}

void pacman_steer(Pacman *pacman, Direction dir)
{
    pacman->wanted = dir;
}

bool pacman_update(Pacman *pacman, const Maze *maze)
{
    if (!entity_tick(&pacman->body)) {
        return false;
    }
    if (can_move(pacman, maze, pacman->wanted)) {
        pacman->body.dir = pacman->wanted;
    }
    if (!can_move(pacman, maze, pacman->body.dir)) {
        return false;
    }
    entity_move_to(&pacman->body,
                   maze_neighbor(pacman->body.pos, pacman->body.dir),
                   pacman->body.dir);
    return true;
}
