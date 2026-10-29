#include "core/game.h"

#include <stddef.h>

#include "core/config.h"

/* Segundos que cada fantasma espera en la casa antes de salir. */
static const int GHOST_RELEASE_SECONDS[GHOST_COUNT] = { 0, 1, 4, 7 };

/* =====================================================================
 * Reglas auxiliares
 * ===================================================================== */

static GhostSpeeds ghost_speeds_for_level(int level)
{
    GhostSpeeds speeds;
    speeds.normal = level < 3 ? GHOST_MOVE_INTERVAL : GHOST_FAST_MOVE_INTERVAL;
    speeds.frightened = GHOST_FRIGHTENED_INTERVAL;
    speeds.eaten = GHOST_EATEN_INTERVAL;
    speeds.tunnel = GHOST_TUNNEL_INTERVAL;
    return speeds;
}

static int frightened_ticks_for_level(int level)
{
    int seconds = FRIGHTENED_SECONDS - (level - 1);
    if (seconds < FRIGHTENED_MIN_SECONDS) {
        seconds = FRIGHTENED_MIN_SECONDS;
    }
    return SECONDS_TO_TICKS(seconds);
}

static void reset_actors(Game *game)
{
    pacman_spawn(&game->pacman, game->maze.pacman_spawn);
    for (int i = 0; i < GHOST_COUNT; ++i) {
        ghost_create(&game->ghosts[i], (GhostId)i, game->maze.ghost_spawns[i],
                     &game->maze, SECONDS_TO_TICKS(GHOST_RELEASE_SECONDS[i]));
    }
    mode_schedule_reset(&game->schedule);
    game->frightened_ticks = 0;
    game->ghosts_eaten_combo = 0;
}

/* =====================================================================
 * PATRÓN STATE para las fases del juego.
 * Cada fase sabe qué hacer al entrar y en cada tick; game_tick() sólo delega.
 * ===================================================================== */

typedef struct {
    void (*enter)(Game *game);
    void (*tick)(Game *game);
} PhaseBehavior;

static const PhaseBehavior *phase_behavior(GamePhase phase);

static void change_phase(Game *game, GamePhase phase)
{
    game->phase = phase;
    game->phase_ticks = 0;
    phase_behavior(phase)->enter(game);
}

static bool countdown_finished(Game *game)
{
    return --game->phase_ticks <= 0;
}

static void start_level(Game *game)
{
    maze_load_classic(&game->maze);
    game->ghost_speeds = ghost_speeds_for_level(game->level);
    reset_actors(game);
    change_phase(game, PHASE_READY);
}

/* ---------- READY ---------- */

static void ready_enter(Game *game)
{
    game->phase_ticks = SECONDS_TO_TICKS(READY_SECONDS);
}

static void ready_tick(Game *game)
{
    if (countdown_finished(game)) {
        change_phase(game, PHASE_PLAYING);
    }
}

/* ---------- PLAYING ---------- */

static void start_frightened(Game *game)
{
    game->frightened_ticks = frightened_ticks_for_level(game->level);
    game->ghosts_eaten_combo = 0;
    for (int i = 0; i < GHOST_COUNT; ++i) {
        ghost_frighten(&game->ghosts[i]);
    }
}

static void eat_current_tile(Game *game)
{
    switch (maze_consume(&game->maze, game->pacman.body.pos)) {
    case TILE_PELLET:
        event_bus_publish(&game->events, EVENT_PELLET_EATEN, 1);
        break;
    case TILE_POWER:
        event_bus_publish(&game->events, EVENT_POWER_EATEN, 1);
        start_frightened(game);
        break;
    default:
        return;
    }
    if (maze_pellets_left(&game->maze) == 0) {
        change_phase(game, PHASE_LEVEL_CLEAR);
    }
}

static void eat_ghost(Game *game, Ghost *ghost)
{
    const int points = SCORE_FIRST_GHOST << game->ghosts_eaten_combo;
    game->ghosts_eaten_combo++;
    ghost_get_eaten(ghost);
    event_bus_publish(&game->events, EVENT_GHOST_EATEN, points);
}

static void resolve_collisions(Game *game)
{
    for (int i = 0; i < GHOST_COUNT && game->phase == PHASE_PLAYING; ++i) {
        Ghost *ghost = &game->ghosts[i];
        if (!vec2_equals(ghost->body.pos, game->pacman.body.pos)) {
            continue;
        }
        if (ghost_is_edible(ghost)) {
            eat_ghost(game, ghost);
        } else if (ghost_is_lethal(ghost)) {
            change_phase(game, PHASE_DYING);
        }
    }
}

static void reverse_all_ghosts(Game *game)
{
    for (int i = 0; i < GHOST_COUNT; ++i) {
        ghost_reverse(&game->ghosts[i]);
    }
}

static void calm_all_ghosts(Game *game)
{
    for (int i = 0; i < GHOST_COUNT; ++i) {
        ghost_calm(&game->ghosts[i]);
    }
}

/* Mientras dura el susto, el cronograma de modos queda en pausa (como en el arcade). */
static void update_ghost_mode(Game *game)
{
    if (game->frightened_ticks > 0) {
        if (--game->frightened_ticks == 0) {
            calm_all_ghosts(game);
        }
        return;
    }
    if (mode_schedule_tick(&game->schedule)) {
        reverse_all_ghosts(game);
    }
}

static void update_ghosts(Game *game)
{
    const ChaseContext chase = {
        game->pacman.body.pos,
        game->pacman.body.dir,
        game->ghosts[GHOST_BLINKY].body.pos,
    };
    const GhostWorld world = {
        &game->maze,
        mode_schedule_current(&game->schedule),
        &chase,
        &game->ghost_speeds,
        &game->rng,
    };
    for (int i = 0; i < GHOST_COUNT; ++i) {
        ghost_update(&game->ghosts[i], &world);
    }
}

static void playing_enter(Game *game)
{
    (void)game;
}

static void playing_tick(Game *game)
{
    if (pacman_update(&game->pacman, &game->maze)) {
        eat_current_tile(game);
    }
    /* Se chequea antes y después de mover a los fantasmas para que no se "atraviesen". */
    resolve_collisions(game);
    if (game->phase != PHASE_PLAYING) {
        return;
    }
    update_ghost_mode(game);
    update_ghosts(game);
    resolve_collisions(game);
}

/* ---------- DYING ---------- */

static void dying_enter(Game *game)
{
    game->phase_ticks = SECONDS_TO_TICKS(DYING_SECONDS);
    event_bus_publish(&game->events, EVENT_PACMAN_DIED, 1);
}

static void dying_tick(Game *game)
{
    if (!countdown_finished(game)) {
        return;
    }
    game->lives--;
    if (game->lives <= 0) {
        change_phase(game, PHASE_GAME_OVER);
        return;
    }
    reset_actors(game);
    change_phase(game, PHASE_READY);
}

/* ---------- LEVEL_CLEAR ---------- */

static void level_clear_enter(Game *game)
{
    game->phase_ticks = SECONDS_TO_TICKS(LEVEL_CLEAR_SECONDS);
    event_bus_publish(&game->events, EVENT_LEVEL_CLEARED, game->level);
}

static void level_clear_tick(Game *game)
{
    if (countdown_finished(game)) {
        game->level++;
        start_level(game);
    }
}

/* ---------- GAME_OVER ---------- */

static void game_over_enter(Game *game)
{
    (void)game;
}

static void game_over_tick(Game *game)
{
    (void)game;
}

static const PhaseBehavior PHASES[PHASE_COUNT] = {
    [PHASE_READY]       = { ready_enter,       ready_tick       },
    [PHASE_PLAYING]     = { playing_enter,     playing_tick     },
    [PHASE_DYING]       = { dying_enter,       dying_tick       },
    [PHASE_LEVEL_CLEAR] = { level_clear_enter, level_clear_tick },
    [PHASE_GAME_OVER]   = { game_over_enter,   game_over_tick   },
};

static const PhaseBehavior *phase_behavior(GamePhase phase)
{
    return &PHASES[phase];
}

/* =====================================================================
 * Observer: el propio juego escucha las vidas extra que otorga el marcador.
 * ===================================================================== */

static void game_on_event(void *listener, const GameEvent *event)
{
    Game *game = (Game *)listener;
    if (event->type == EVENT_EXTRA_LIFE) {
        game->lives += event->value;
    }
}

/* =====================================================================
 * PATRÓN COMMAND: cada orden del jugador se resuelve con un handler de tabla.
 * ===================================================================== */

typedef void (*CommandHandler)(Game *game);

static bool accepts_steering(const Game *game)
{
    return !game->paused && (game->phase == PHASE_PLAYING || game->phase == PHASE_READY);
}

static void steer(Game *game, Direction dir)
{
    if (accepts_steering(game)) {
        pacman_steer(&game->pacman, dir);
    }
}

static void cmd_up(Game *game)    { steer(game, DIR_UP); }
static void cmd_down(Game *game)  { steer(game, DIR_DOWN); }
static void cmd_left(Game *game)  { steer(game, DIR_LEFT); }
static void cmd_right(Game *game) { steer(game, DIR_RIGHT); }

static void cmd_pause(Game *game)
{
    if (game->phase != PHASE_GAME_OVER) {
        game->paused = !game->paused;
    }
}

static void init_session(Game *game, uint32_t seed, int high_score);

static void cmd_restart(Game *game)
{
    if (game->phase == PHASE_GAME_OVER) {
        init_session(game, rng_next(&game->rng), game->scoreboard.high_score);
    }
}

static void cmd_quit(Game *game)
{
    game->quit_requested = true;
}

static const CommandHandler COMMAND_HANDLERS[CMD_COUNT] = {
    [CMD_UP]      = cmd_up,
    [CMD_DOWN]    = cmd_down,
    [CMD_LEFT]    = cmd_left,
    [CMD_RIGHT]   = cmd_right,
    [CMD_PAUSE]   = cmd_pause,
    [CMD_RESTART] = cmd_restart,
    [CMD_QUIT]    = cmd_quit,
};

/* =====================================================================
 * API pública
 * ===================================================================== */

static void init_session(Game *game, uint32_t seed, int high_score)
{
    rng_seed(&game->rng, seed);
    event_bus_init(&game->events);
    scoreboard_init(&game->scoreboard, &game->events, high_score);
    event_bus_subscribe(&game->events, scoreboard_on_event, &game->scoreboard);
    event_bus_subscribe(&game->events, game_on_event, game);

    game->lives = START_LIVES;
    game->level = 1;
    game->frame = 0;
    game->paused = false;
    game->quit_requested = false;
    start_level(game);
}

void game_init(Game *game, uint32_t seed)
{
    init_session(game, seed, 0);
}

void game_handle_command(Game *game, Command command)
{
    if (command <= CMD_NONE || command >= CMD_COUNT) {
        return;
    }
    const CommandHandler handler = COMMAND_HANDLERS[command];
    if (handler != NULL) {
        handler(game);
    }
}

void game_tick(Game *game)
{
    if (game->paused || game->quit_requested) {
        return;
    }
    game->frame++;
    phase_behavior(game->phase)->tick(game);
}

bool game_should_quit(const Game *game)
{
    return game->quit_requested;
}

bool game_frightened_is_ending(const Game *game)
{
    return game->frightened_ticks > 0
        && game->frightened_ticks < SECONDS_TO_TICKS(FRIGHTENED_FLASH_SECONDS);
}
