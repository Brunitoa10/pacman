#ifndef PACMAN_CORE_GHOST_AI_H
#define PACMAN_CORE_GHOST_AI_H

#include "core/direction.h"
#include "core/vec2.h"

typedef enum {
    GHOST_BLINKY,
    GHOST_PINKY,
    GHOST_INKY,
    GHOST_CLYDE,
    GHOST_COUNT
} GhostId;

/* Información que una estrategia de persecución necesita del mundo. */
typedef struct {
    Vec2 pacman_pos;
    Direction pacman_dir;
    Vec2 blinky_pos;
} ChaseContext;

/*
 * PATRÓN STRATEGY: cada fantasma difiere sólo en cómo elige su objetivo.
 * Agregar un fantasma nuevo = escribir una función y una entrada en la tabla;
 * el resto del código (ghost.c) no cambia.
 */
typedef Vec2 (*ChaseTargetFn)(Vec2 self, const ChaseContext *ctx);

typedef struct {
    const char *name;
    Vec2 scatter_target;
    ChaseTargetFn chase_target;
} GhostPersonality;

const GhostPersonality *ghost_ai_personality(GhostId id);

#endif /* PACMAN_CORE_GHOST_AI_H */
