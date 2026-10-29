#include "core/ghost.h"

#include <limits.h>

/*
 * PATRÓN STATE (dirigido por tabla): cada estado del fantasma define cómo se
 * comporta. ghost_update() no tiene un switch gigante: delega en la tabla.
 * Agregar un estado nuevo = agregar una fila.
 */
typedef struct {
    bool can_pass_door;
    bool moves_randomly;
    Vec2 (*target)(const Ghost *ghost, const GhostWorld *world);
    int (*interval)(const Ghost *ghost, const GhostWorld *world);
    void (*on_arrival)(Ghost *ghost, const GhostWorld *world);
} GhostStateBehavior;

/* ---------- objetivos ---------- */

static Vec2 target_house_exit(const Ghost *ghost, const GhostWorld *world)
{
    (void)ghost;
    return world->maze->house_exit;
}

static Vec2 target_house_center(const Ghost *ghost, const GhostWorld *world)
{
    (void)ghost;
    return world->maze->house_center;
}

static Vec2 target_by_mode(const Ghost *ghost, const GhostWorld *world)
{
    if (world->mode == MODE_SCATTER) {
        return ghost->personality->scatter_target;
    }
    return ghost->personality->chase_target(ghost->body.pos, world->chase);
}

/* ---------- velocidades ---------- */

static int interval_normal(const Ghost *ghost, const GhostWorld *world)
{
    return maze_is_tunnel(world->maze, ghost->body.pos)
         ? world->speeds->tunnel
         : world->speeds->normal;
}

static int interval_frightened(const Ghost *ghost, const GhostWorld *world)
{
    (void)ghost;
    return world->speeds->frightened;
}

static int interval_eaten(const Ghost *ghost, const GhostWorld *world)
{
    (void)ghost;
    return world->speeds->eaten;
}

/* ---------- transiciones al llegar a una casilla ---------- */

static void arrival_none(Ghost *ghost, const GhostWorld *world)
{
    (void)ghost;
    (void)world;
}

static void arrival_leaving(Ghost *ghost, const GhostWorld *world)
{
    if (vec2_equals(ghost->body.pos, world->maze->house_exit)) {
        ghost->state = GHOST_ROAMING;
    }
}

static void arrival_eaten(Ghost *ghost, const GhostWorld *world)
{
    if (vec2_equals(ghost->body.pos, world->maze->house_exit)) {
        ghost->state = GHOST_ENTERING_HOUSE;
    }
}

static void arrival_entering(Ghost *ghost, const GhostWorld *world)
{
    if (vec2_equals(ghost->body.pos, world->maze->house_center)) {
        ghost->state = GHOST_LEAVING_HOUSE;
        ghost->body.dir = DIR_NONE; /* puede dar media vuelta para salir */
    }
}

static const GhostStateBehavior BEHAVIORS[GHOST_STATE_COUNT] = {
    [GHOST_IN_HOUSE]       = { false, false, target_house_center, interval_normal,     arrival_none     },
    [GHOST_LEAVING_HOUSE]  = { true,  false, target_house_exit,   interval_normal,     arrival_leaving  },
    [GHOST_ROAMING]        = { false, false, target_by_mode,      interval_normal,     arrival_none     },
    [GHOST_FRIGHTENED]     = { false, true,  target_by_mode,      interval_frightened, arrival_none     },
    [GHOST_EATEN]          = { true,  false, target_house_exit,   interval_eaten,      arrival_eaten    },
    [GHOST_ENTERING_HOUSE] = { true,  false, target_house_center, interval_eaten,      arrival_entering },
};

/* ---------- elección de dirección ---------- */

static bool can_take(const Ghost *ghost, const GhostWorld *world, Direction dir)
{
    if (dir == DIR_NONE || dir == direction_opposite(ghost->body.dir)) {
        return false; /* los fantasmas no dan media vuelta por su cuenta */
    }
    const Vec2 next = maze_neighbor(ghost->body.pos, dir);
    return maze_is_walkable(world->maze, next, BEHAVIORS[ghost->state].can_pass_door);
}

static Direction choose_towards(const Ghost *ghost, const GhostWorld *world, Vec2 target)
{
    Direction best = DIR_NONE;
    int best_distance = INT_MAX;

    for (int d = DIR_UP; d < DIR_COUNT; ++d) {
        const Direction dir = (Direction)d;
        if (!can_take(ghost, world, dir)) {
            continue;
        }
        const int distance = vec2_distance_sq(maze_neighbor(ghost->body.pos, dir), target);
        if (distance < best_distance) {
            best_distance = distance;
            best = dir;
        }
    }
    return best;
}

static Direction choose_randomly(const Ghost *ghost, const GhostWorld *world)
{
    Direction options[DIR_COUNT];
    int count = 0;

    for (int d = DIR_UP; d < DIR_COUNT; ++d) {
        if (can_take(ghost, world, (Direction)d)) {
            options[count++] = (Direction)d;
        }
    }
    return count > 0 ? options[rng_range(world->rng, count)] : DIR_NONE;
}

static Direction choose_direction(const Ghost *ghost, const GhostWorld *world)
{
    const GhostStateBehavior *behavior = &BEHAVIORS[ghost->state];
    const Direction dir = behavior->moves_randomly
                        ? choose_randomly(ghost, world)
                        : choose_towards(ghost, world, behavior->target(ghost, world));

    /* Callejón sin salida: única opción es volver. */
    return dir != DIR_NONE ? dir : direction_opposite(ghost->body.dir);
}

/* ---------- API pública ---------- */

void ghost_create(Ghost *ghost, GhostId id, Vec2 spawn, const Maze *maze, int release_ticks)
{
    const bool starts_outside = spawn.row <= maze->house_exit.row;

    entity_place(&ghost->body, spawn, starts_outside ? DIR_LEFT : DIR_NONE, GHOST_MOVE_INTERVAL);
    ghost->id = id;
    ghost->personality = ghost_ai_personality(id);
    ghost->state = starts_outside ? GHOST_ROAMING : GHOST_IN_HOUSE;
    ghost->release_ticks = release_ticks;
}

static void wait_in_house(Ghost *ghost)
{
    if (ghost->release_ticks > 0) {
        ghost->release_ticks--;
    } else {
        ghost->state = GHOST_LEAVING_HOUSE;
    }
}

void ghost_update(Ghost *ghost, const GhostWorld *world)
{
    if (ghost->state == GHOST_IN_HOUSE) {
        wait_in_house(ghost);
        return;
    }

    const GhostStateBehavior *behavior = &BEHAVIORS[ghost->state];
    entity_set_interval(&ghost->body, behavior->interval(ghost, world));
    if (!entity_tick(&ghost->body)) {
        return;
    }

    const Direction dir = choose_direction(ghost, world);
    if (dir == DIR_NONE) {
        return;
    }
    const Vec2 next = maze_neighbor(ghost->body.pos, dir);
    if (!maze_is_walkable(world->maze, next, behavior->can_pass_door)) {
        return;
    }
    entity_move_to(&ghost->body, next, dir);
    behavior->on_arrival(ghost, world);
}

void ghost_reverse(Ghost *ghost)
{
    if (ghost->state == GHOST_ROAMING || ghost->state == GHOST_FRIGHTENED) {
        ghost->body.dir = direction_opposite(ghost->body.dir);
    }
}

void ghost_frighten(Ghost *ghost)
{
    if (ghost->state == GHOST_ROAMING) {
        ghost_reverse(ghost);
        ghost->state = GHOST_FRIGHTENED;
    }
}

void ghost_calm(Ghost *ghost)
{
    if (ghost->state == GHOST_FRIGHTENED) {
        ghost->state = GHOST_ROAMING;
    }
}

void ghost_get_eaten(Ghost *ghost)
{
    ghost->state = GHOST_EATEN;
}

bool ghost_is_edible(const Ghost *ghost)
{
    return ghost->state == GHOST_FRIGHTENED;
}

bool ghost_is_lethal(const Ghost *ghost)
{
    return ghost->state == GHOST_ROAMING;
}
