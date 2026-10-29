#ifndef PACMAN_CORE_GAME_H
#define PACMAN_CORE_GAME_H

#include <stdbool.h>
#include <stdint.h>

#include "core/command.h"
#include "core/event_bus.h"
#include "core/ghost.h"
#include "core/maze.h"
#include "core/mode_schedule.h"
#include "core/pacman.h"
#include "core/rng.h"
#include "core/scoreboard.h"

typedef enum {
    PHASE_READY,
    PHASE_PLAYING,
    PHASE_DYING,
    PHASE_LEVEL_CLEAR,
    PHASE_GAME_OVER,
    PHASE_COUNT
} GamePhase;

/*
 * Agregado raíz del dominio. No hace I/O: no dibuja, no lee teclado ni mide
 * tiempo. Eso lo hace la capa de plataforma, que depende de esto y no al revés.
 *
 * Fuera de game.c, tratar los campos como de sólo lectura (la vista los lee).
 * Importante: no copiar ni mover un Game ya inicializado (el bus guarda punteros
 * a sus miembros).
 */
typedef struct Game {
    Maze maze;
    Pacman pacman;
    Ghost ghosts[GHOST_COUNT];
    ModeSchedule schedule;
    GhostSpeeds ghost_speeds;
    Rng rng;
    EventBus events;
    Scoreboard scoreboard;

    GamePhase phase;
    int phase_ticks;
    int frightened_ticks;
    int ghosts_eaten_combo;
    int lives;
    int level;
    unsigned long frame;
    bool paused;
    bool quit_requested;
} Game;

void game_init(Game *game, uint32_t seed);
void game_handle_command(Game *game, Command command);
void game_tick(Game *game);

bool game_should_quit(const Game *game);
bool game_frightened_is_ending(const Game *game);

#endif /* PACMAN_CORE_GAME_H */
