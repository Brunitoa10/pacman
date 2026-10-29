#ifndef PACMAN_CORE_GHOST_H
#define PACMAN_CORE_GHOST_H

#include <stdbool.h>

#include "core/entity.h"
#include "core/ghost_ai.h"
#include "core/maze.h"
#include "core/mode_schedule.h"
#include "core/rng.h"

typedef enum {
    GHOST_IN_HOUSE,      /* esperando ser liberado */
    GHOST_LEAVING_HOUSE, /* saliendo por la puerta */
    GHOST_ROAMING,       /* dispersión o persecución, según el modo global */
    GHOST_FRIGHTENED,    /* azul: se lo puede comer */
    GHOST_EATEN,         /* "ojos" volviendo a la casa */
    GHOST_ENTERING_HOUSE,/* ojos cruzando la puerta hacia adentro */
    GHOST_STATE_COUNT
} GhostState;

/* Velocidades en ticks por casilla, dependen del nivel. */
typedef struct {
    int normal;
    int frightened;
    int eaten;
    int tunnel;
} GhostSpeeds;

/*
 * PATRÓN PARAMETER OBJECT: todo lo que un fantasma necesita saber del mundo
 * en un tick, agrupado en una sola estructura.
 */
typedef struct {
    const Maze *maze;
    GhostMode mode;
    const ChaseContext *chase;
    const GhostSpeeds *speeds;
    Rng *rng;
} GhostWorld;

typedef struct {
    Entity body;
    GhostId id;
    GhostState state;
    const GhostPersonality *personality; /* estrategia inyectada */
    int release_ticks;
} Ghost;

/* PATRÓN FACTORY: crea un fantasma con su personalidad y estado inicial. */
void ghost_create(Ghost *ghost, GhostId id, Vec2 spawn, const Maze *maze, int release_ticks);

void ghost_update(Ghost *ghost, const GhostWorld *world);

void ghost_frighten(Ghost *ghost);
void ghost_calm(Ghost *ghost);
void ghost_get_eaten(Ghost *ghost);
void ghost_reverse(Ghost *ghost);

bool ghost_is_edible(const Ghost *ghost);
bool ghost_is_lethal(const Ghost *ghost);

#endif /* PACMAN_CORE_GHOST_H */
