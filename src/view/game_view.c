#include "view/game_view.h"

#include <stdio.h>

#define HUD_ROW 0
#define MAZE_TOP 1
#define FOOTER_ROW (MAZE_TOP + MAZE_ROWS)
#define MESSAGE_ROW (MAZE_TOP + 17) /* zona libre bajo la casa de los fantasmas */
#define TEXT_BUFFER 64

static const Glyph PACMAN_GLYPHS[DIR_COUNT] = {
    [DIR_UP]    = GLYPH_PACMAN_UP,
    [DIR_LEFT]  = GLYPH_PACMAN_LEFT,
    [DIR_DOWN]  = GLYPH_PACMAN_DOWN,
    [DIR_RIGHT] = GLYPH_PACMAN_RIGHT,
};

static const Glyph GHOST_GLYPHS[GHOST_COUNT] = {
    [GHOST_BLINKY] = GLYPH_GHOST_BLINKY,
    [GHOST_PINKY]  = GLYPH_GHOST_PINKY,
    [GHOST_INKY]   = GLYPH_GHOST_INKY,
    [GHOST_CLYDE]  = GLYPH_GHOST_CLYDE,
};

int game_view_rows(void)
{
    return FOOTER_ROW + 1;
}

int game_view_cols(void)
{
    return MAZE_COLS;
}

static bool blink(const Game *game, unsigned long period)
{
    return (game->frame / period) % 2 == 0;
}

static Glyph tile_glyph(Tile tile)
{
    switch (tile) {
    case TILE_WALL:   return GLYPH_WALL;
    case TILE_DOOR:   return GLYPH_DOOR;
    case TILE_PELLET: return GLYPH_PELLET;
    case TILE_POWER:  return GLYPH_POWER;
    default:          return GLYPH_EMPTY;
    }
}

static void draw_maze(const Game *game, const Renderer *r)
{
    const bool hide_power = !blink(game, 8); /* los power pellets titilan */

    for (int row = 0; row < MAZE_ROWS; ++row) {
        for (int col = 0; col < MAZE_COLS; ++col) {
            const Tile tile = game->maze.tiles[row][col];
            const Glyph glyph = (tile == TILE_POWER && hide_power) ? GLYPH_EMPTY : tile_glyph(tile);
            r->draw_cell(r->ctx, MAZE_TOP + row, col, glyph);
        }
    }
}

static Glyph ghost_glyph(const Game *game, const Ghost *ghost)
{
    switch (ghost->state) {
    case GHOST_EATEN:
    case GHOST_ENTERING_HOUSE:
        return GLYPH_GHOST_EYES;
    case GHOST_FRIGHTENED:
        return (game_frightened_is_ending(game) && !blink(game, 6))
             ? GLYPH_GHOST_FLASHING
             : GLYPH_GHOST_FRIGHTENED;
    default:
        return GHOST_GLYPHS[ghost->id];
    }
}

static void draw_ghosts(const Game *game, const Renderer *r)
{
    for (int i = 0; i < GHOST_COUNT; ++i) {
        const Ghost *ghost = &game->ghosts[i];
        r->draw_cell(r->ctx, MAZE_TOP + ghost->body.pos.row, ghost->body.pos.col,
                     ghost_glyph(game, ghost));
    }
}

static Glyph pacman_glyph(const Game *game)
{
    const Direction dir = game->pacman.body.dir;
    if (dir == DIR_NONE || !blink(game, 4)) {
        return GLYPH_PACMAN_CLOSED;
    }
    return PACMAN_GLYPHS[dir];
}

static void draw_pacman(const Game *game, const Renderer *r)
{
    if (game->phase == PHASE_DYING && !blink(game, 5)) {
        return;
    }
    r->draw_cell(r->ctx, MAZE_TOP + game->pacman.body.pos.row, game->pacman.body.pos.col,
                 pacman_glyph(game));
}

static void draw_hud(const Game *game, const Renderer *r)
{
    char text[TEXT_BUFFER];

    snprintf(text, sizeof text, "PUNTOS %06d", game->scoreboard.score);
    r->draw_text(r->ctx, HUD_ROW, ALIGN_LEFT, TEXT_NORMAL, text);

    snprintf(text, sizeof text, "RECORD %06d", game->scoreboard.high_score);
    r->draw_text(r->ctx, HUD_ROW, ALIGN_CENTER, TEXT_HIGHLIGHT, text);

    snprintf(text, sizeof text, "NIVEL %02d", game->level);
    r->draw_text(r->ctx, HUD_ROW, ALIGN_RIGHT, TEXT_NORMAL, text);
}

static void draw_footer(const Game *game, const Renderer *r)
{
    char text[TEXT_BUFFER];
    snprintf(text, sizeof text, "VIDAS: %d", game->lives > 0 ? game->lives : 0);
    r->draw_text(r->ctx, FOOTER_ROW, ALIGN_LEFT, TEXT_HIGHLIGHT, text);

    const char *help = (game->phase == PHASE_GAME_OVER)
                     ? "R: reiniciar  Q: salir"
                     : "WASD/flechas  P: pausa  Q: salir";
    r->draw_text(r->ctx, FOOTER_ROW, ALIGN_RIGHT, TEXT_NORMAL, help);
}

static void draw_message(const Game *game, const Renderer *r)
{
    if (game->paused) {
        r->draw_text(r->ctx, MESSAGE_ROW, ALIGN_CENTER, TEXT_HIGHLIGHT, "PAUSA");
        return;
    }
    switch (game->phase) {
    case PHASE_READY:
        r->draw_text(r->ctx, MESSAGE_ROW, ALIGN_CENTER, TEXT_HIGHLIGHT, "READY!");
        break;
    case PHASE_LEVEL_CLEAR:
        r->draw_text(r->ctx, MESSAGE_ROW, ALIGN_CENTER, TEXT_HIGHLIGHT, "NIVEL COMPLETO");
        break;
    case PHASE_GAME_OVER:
        r->draw_text(r->ctx, MESSAGE_ROW, ALIGN_CENTER, TEXT_WARNING, "GAME OVER");
        break;
    default:
        break;
    }
}

static bool shows_actors(const Game *game)
{
    return game->phase != PHASE_GAME_OVER && game->phase != PHASE_LEVEL_CLEAR;
}

void game_view_render(const Game *game, const Renderer *renderer)
{
    renderer->begin_frame(renderer->ctx);

    draw_hud(game, renderer);
    draw_maze(game, renderer);
    if (shows_actors(game)) {
        if (game->phase != PHASE_DYING) {
            draw_ghosts(game, renderer);
        }
        draw_pacman(game, renderer);
    }
    draw_message(game, renderer);
    draw_footer(game, renderer);

    renderer->end_frame(renderer->ctx);
}
