#include "core/ghost_ai.h"

#include "core/config.h"

#define BLINKY_CORNER { -3, MAZE_COLS - 3 }
#define PINKY_CORNER  { -3, 2 }
#define INKY_CORNER   { MAZE_ROWS, MAZE_COLS - 1 }
#define CLYDE_CORNER  { MAZE_ROWS, 0 }

static Vec2 ahead_of_pacman(const ChaseContext *ctx, int tiles)
{
    return vec2_add(ctx->pacman_pos, vec2_scale(direction_to_vec(ctx->pacman_dir), tiles));
}

/* Blinky: persigue directamente a Pac-Man. */
static Vec2 blinky_chase(Vec2 self, const ChaseContext *ctx)
{
    (void)self;
    return ctx->pacman_pos;
}

/* Pinky: embosca apuntando 4 casillas delante de Pac-Man. */
static Vec2 pinky_chase(Vec2 self, const ChaseContext *ctx)
{
    (void)self;
    return ahead_of_pacman(ctx, 4);
}

/* Inky: refleja la posición de Blinky respecto de un punto 2 casillas delante de Pac-Man. */
static Vec2 inky_chase(Vec2 self, const ChaseContext *ctx)
{
    (void)self;
    const Vec2 pivot = ahead_of_pacman(ctx, 2);
    return vec2_add(pivot, vec2_sub(pivot, ctx->blinky_pos));
}

/* Clyde: persigue de lejos, pero si se acerca demasiado huye a su esquina. */
static Vec2 clyde_chase(Vec2 self, const ChaseContext *ctx)
{
    static const Vec2 corner = CLYDE_CORNER;
    if (vec2_distance_sq(self, ctx->pacman_pos) > CLYDE_SHY_DISTANCE_SQ) {
        return ctx->pacman_pos;
    }
    return corner;
}

static const GhostPersonality PERSONALITIES[GHOST_COUNT] = {
    [GHOST_BLINKY] = { "Blinky", BLINKY_CORNER, blinky_chase },
    [GHOST_PINKY]  = { "Pinky",  PINKY_CORNER,  pinky_chase  },
    [GHOST_INKY]   = { "Inky",   INKY_CORNER,   inky_chase   },
    [GHOST_CLYDE]  = { "Clyde",  CLYDE_CORNER,  clyde_chase  },
};

const GhostPersonality *ghost_ai_personality(GhostId id)
{
    if (id < 0 || id >= GHOST_COUNT) {
        return &PERSONALITIES[GHOST_BLINKY];
    }
    return &PERSONALITIES[id];
}
