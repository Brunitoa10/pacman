/*
 * Tests unitarios del núcleo. Compilan SIN la capa de plataforma: prueba de
 * que la lógica del juego no depende de la consola (Inversión de Dependencias).
 */
#include <stdio.h>
#include <string.h>

#include "core/config.h"
#include "core/game.h"
#include "core/ghost.h"
#include "core/ghost_ai.h"
#include "core/maze.h"
#include "core/mode_schedule.h"
#include "core/pacman.h"
#include "ports/renderer.h"
#include "view/game_view.h"

static int tests_run = 0;
static int tests_failed = 0;

#define CHECK(cond)                                                          \
    do {                                                                     \
        tests_run++;                                                         \
        if (!(cond)) {                                                       \
            tests_failed++;                                                  \
            printf("  FALLO %s:%d: %s\n", __FILE__, __LINE__, #cond);        \
        }                                                                    \
    } while (0)

/* ---------- Maze ---------- */

static void test_classic_maze_loads(void)
{
    Maze maze;
    maze_load_classic(&maze);
    CHECK(maze_pellets_left(&maze) == 244);
    CHECK(vec2_equals(maze.pacman_spawn, vec2_make(23, 13)));
    CHECK(vec2_equals(maze.house_exit, vec2_make(11, 13)));
    CHECK(vec2_equals(maze.house_center, vec2_make(14, 13)));
    CHECK(maze_tile_at(&maze, vec2_make(0, 0)) == TILE_WALL);
}

static void test_tunnel_wraps(void)
{
    Maze maze;
    maze_load_classic(&maze);
    const Vec2 left_edge = vec2_make(14, 0);
    CHECK(vec2_equals(maze_neighbor(left_edge, DIR_LEFT), vec2_make(14, MAZE_COLS - 1)));
    CHECK(maze_is_tunnel(&maze, left_edge));
    CHECK(!maze_is_tunnel(&maze, vec2_make(1, 1)));
}

static void test_door_only_for_allowed(void)
{
    Maze maze;
    maze_load_classic(&maze);
    const Vec2 door = vec2_make(12, 13);
    CHECK(!maze_is_walkable(&maze, door, false));
    CHECK(maze_is_walkable(&maze, door, true));
}

static void test_consume_pellet(void)
{
    Maze maze;
    maze_load_classic(&maze);
    const int before = maze_pellets_left(&maze);
    CHECK(maze_consume(&maze, vec2_make(1, 1)) == TILE_PELLET);
    CHECK(maze_consume(&maze, vec2_make(1, 1)) == TILE_EMPTY);
    CHECK(maze_pellets_left(&maze) == before - 1);
}

/* ---------- Pacman ---------- */

static void tick_until_move(Pacman *pacman, const Maze *maze)
{
    for (int i = 0; i < PACMAN_MOVE_INTERVAL; ++i) {
        pacman_update(pacman, maze);
    }
}

static void test_pacman_stops_at_wall(void)
{
    Maze maze;
    Pacman pacman;
    maze_load_classic(&maze);
    pacman_spawn(&pacman, vec2_make(1, 1));
    pacman_steer(&pacman, DIR_UP);
    pacman.body.dir = DIR_UP;
    tick_until_move(&pacman, &maze);
    CHECK(vec2_equals(pacman.body.pos, vec2_make(1, 1)));
}

static void test_pacman_buffered_turn(void)
{
    Maze maze;
    Pacman pacman;
    maze_load_classic(&maze);
    pacman_spawn(&pacman, vec2_make(1, 2));
    pacman_steer(&pacman, DIR_DOWN); /* aún no se puede: pared abajo */
    pacman.body.dir = DIR_LEFT;
    tick_until_move(&pacman, &maze);
    CHECK(vec2_equals(pacman.body.pos, vec2_make(1, 1)));
    tick_until_move(&pacman, &maze);
    CHECK(vec2_equals(pacman.body.pos, vec2_make(2, 1)));
}

/* ---------- Estrategias de fantasmas ---------- */

static void test_ghost_strategies(void)
{
    const ChaseContext ctx = { { 10, 10 }, DIR_RIGHT, { 10, 6 } };
    const Vec2 pinky = ghost_ai_personality(GHOST_PINKY)->chase_target(vec2_make(0, 0), &ctx);
    const Vec2 inky = ghost_ai_personality(GHOST_INKY)->chase_target(vec2_make(0, 0), &ctx);
    const Vec2 clyde_far = ghost_ai_personality(GHOST_CLYDE)->chase_target(vec2_make(30, 0), &ctx);
    const Vec2 clyde_near = ghost_ai_personality(GHOST_CLYDE)->chase_target(vec2_make(10, 11), &ctx);

    CHECK(vec2_equals(pinky, vec2_make(10, 14)));
    CHECK(vec2_equals(inky, vec2_make(10, 18)));
    CHECK(vec2_equals(clyde_far, ctx.pacman_pos));
    CHECK(vec2_equals(clyde_near, ghost_ai_personality(GHOST_CLYDE)->scatter_target));
}

/* ---------- Ghost: máquina de estados ---------- */

static GhostWorld make_world(const Maze *maze, const ChaseContext *chase,
                             const GhostSpeeds *speeds, Rng *rng)
{
    const GhostWorld world = { maze, MODE_CHASE, chase, speeds, rng };
    return world;
}

static void test_ghosts_leave_house(void)
{
    Maze maze;
    Rng rng;
    maze_load_classic(&maze);
    rng_seed(&rng, 1);
    const ChaseContext chase = { maze.pacman_spawn, DIR_LEFT, maze.house_exit };
    const GhostSpeeds speeds = { 1, 1, 1, 1 };
    const GhostWorld world = make_world(&maze, &chase, &speeds, &rng);

    for (int id = GHOST_PINKY; id < GHOST_COUNT; ++id) {
        Ghost ghost;
        ghost_create(&ghost, (GhostId)id, maze.ghost_spawns[id], &maze, 0);
        for (int t = 0; t < 50 && ghost.state != GHOST_ROAMING; ++t) {
            ghost_update(&ghost, &world);
        }
        CHECK(ghost.state == GHOST_ROAMING);
    }
}

/* Marca las casillas alcanzables por Pac-Man (las zonas vacías fuera del laberinto no cuentan). */
static void flood_reachable(const Maze *maze, Vec2 pos, bool reachable[MAZE_ROWS][MAZE_COLS])
{
    if (!maze_is_walkable(maze, pos, false) || reachable[pos.row][pos.col]) {
        return;
    }
    reachable[pos.row][pos.col] = true;
    for (int d = DIR_UP; d < DIR_COUNT; ++d) {
        flood_reachable(maze, maze_neighbor(pos, (Direction)d), reachable);
    }
}

/* Robustez: desde CUALQUIER casilla alcanzable, los ojos de un fantasma comido vuelven a casa. */
static void test_eaten_ghost_always_returns_home(void)
{
    Maze maze;
    Rng rng;
    static bool reachable[MAZE_ROWS][MAZE_COLS];
    maze_load_classic(&maze);
    flood_reachable(&maze, maze.pacman_spawn, reachable);
    rng_seed(&rng, 7);
    const ChaseContext chase = { maze.pacman_spawn, DIR_LEFT, maze.house_exit };
    const GhostSpeeds speeds = { 1, 1, 1, 1 };
    const GhostWorld world = make_world(&maze, &chase, &speeds, &rng);
    int stuck = 0;

    for (int r = 0; r < MAZE_ROWS; ++r) {
        for (int c = 0; c < MAZE_COLS; ++c) {
            const Vec2 start = vec2_make(r, c);
            if (!reachable[r][c]) {
                continue;
            }
            for (int d = DIR_UP; d < DIR_COUNT; ++d) {
                Ghost ghost;
                ghost_create(&ghost, GHOST_BLINKY, maze.house_exit, &maze, 0);
                ghost.body.pos = start;
                ghost.body.dir = (Direction)d;
                ghost_get_eaten(&ghost);
                int t = 0;
                while (t++ < 500 && ghost.state != GHOST_ROAMING) {
                    ghost_update(&ghost, &world);
                }
                if (ghost.state != GHOST_ROAMING) {
                    stuck++;
                }
            }
        }
    }
    CHECK(stuck == 0);
}

static void test_frightened_reverses_and_calms(void)
{
    Maze maze;
    Ghost ghost;
    maze_load_classic(&maze);
    ghost_create(&ghost, GHOST_BLINKY, maze.house_exit, &maze, 0);
    ghost.body.dir = DIR_LEFT;
    ghost_frighten(&ghost);
    CHECK(ghost.state == GHOST_FRIGHTENED);
    CHECK(ghost.body.dir == DIR_RIGHT);
    CHECK(ghost_is_edible(&ghost));
    ghost_calm(&ghost);
    CHECK(ghost_is_lethal(&ghost));
}

/* ---------- Cronograma ---------- */

static void test_mode_schedule(void)
{
    ModeSchedule schedule;
    mode_schedule_reset(&schedule);
    CHECK(mode_schedule_current(&schedule) == MODE_SCATTER);
    bool changed = false;
    for (int i = 0; i < SECONDS_TO_TICKS(7); ++i) {
        changed = mode_schedule_tick(&schedule) || changed;
    }
    CHECK(changed);
    CHECK(mode_schedule_current(&schedule) == MODE_CHASE);
}

/* ---------- Observer ---------- */

static void test_scoreboard_observer(void)
{
    static Game game;
    game_init(&game, 42);
    event_bus_publish(&game.events, EVENT_PELLET_EATEN, 1);
    event_bus_publish(&game.events, EVENT_GHOST_EATEN, 400);
    CHECK(game.scoreboard.score == SCORE_PELLET + 400);

    const int lives = game.lives;
    event_bus_publish(&game.events, EVENT_GHOST_EATEN, EXTRA_LIFE_SCORE);
    CHECK(game.lives == lives + 1);
}

/* ---------- Simulación completa con un "bot" y un renderer falso ---------- */

typedef struct {
    int cells;
    int frames;
} FakeRenderer;

static void fake_begin(void *ctx) { ((FakeRenderer *)ctx)->frames++; }
static void fake_cell(void *ctx, int r, int c, Glyph g) { (void)r; (void)c; (void)g; ((FakeRenderer *)ctx)->cells++; }
static void fake_text(void *ctx, int r, TextAlign a, TextStyle s, const char *t) { (void)ctx; (void)r; (void)a; (void)s; (void)t; }
static void fake_end(void *ctx) { (void)ctx; }
static void fake_destroy(void *ctx) { (void)ctx; }

static void test_full_game_simulation(void)
{
    static Game game;
    FakeRenderer fake = { 0, 0 };
    const Renderer renderer = { &fake, fake_begin, fake_cell, fake_text, fake_end, fake_destroy };
    Rng bot;
    rng_seed(&bot, 99);
    game_init(&game, 123);

    for (int t = 0; t < 60 * TICKS_PER_SECOND && game.phase != PHASE_GAME_OVER; ++t) {
        if (t % 20 == 0) {
            game_handle_command(&game, (Command)(CMD_UP + rng_range(&bot, 4)));
        }
        game_tick(&game);
        game_view_render(&game, &renderer);
    }
    CHECK(fake.frames > 0);
    CHECK(game.scoreboard.score > 0);
    CHECK(maze_pellets_left(&game.maze) < 244);
}

int main(void)
{
    test_classic_maze_loads();
    test_tunnel_wraps();
    test_door_only_for_allowed();
    test_consume_pellet();
    test_pacman_stops_at_wall();
    test_pacman_buffered_turn();
    test_ghost_strategies();
    test_ghosts_leave_house();
    test_eaten_ghost_always_returns_home();
    test_frightened_reverses_and_calms();
    test_mode_schedule();
    test_scoreboard_observer();
    test_full_game_simulation();

    printf("%d checks, %d fallidos\n", tests_run, tests_failed);
    return tests_failed == 0 ? 0 : 1;
}
