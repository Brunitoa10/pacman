#include "platform/raylib_renderer.h"

#include <stdlib.h>
#include <string.h>

#include "raylib.h"

#define CELL_SIZE 22
#define FONT_SIZE 18
#define TEXT_MARGIN 6
#define MAX_TEXTS 16
#define MAX_TEXT_LEN 64
#define WALL_THICKNESS 2.0f
#define WALL_INSET (CELL_SIZE * 0.35f)
#define CIRCLE_SEGMENTS 24

/* ---------- Paleta ---------- */

static const Color BACKGROUND   = { 0, 0, 0, 255 };
static const Color WALL_COLOR   = { 33, 33, 222, 255 };
static const Color DOOR_COLOR   = { 255, 184, 222, 255 };
static const Color PELLET_COLOR = { 255, 184, 151, 255 };
static const Color PACMAN_COLOR = { 255, 255, 0, 255 };
static const Color EYE_WHITE    = { 255, 255, 255, 255 };
static const Color PUPIL_COLOR  = { 33, 33, 222, 255 };
static const Color SCARED_BLUE  = { 33, 33, 255, 255 };
static const Color SCARED_FACE  = { 255, 184, 151, 255 };
static const Color FLASH_WHITE  = { 222, 222, 255, 255 };
static const Color FLASH_FACE   = { 255, 0, 0, 255 };

static const Color GHOST_COLORS[] = {
    { 255, 0, 0, 255 },     /* Blinky */
    { 255, 184, 255, 255 }, /* Pinky */
    { 0, 255, 255, 255 },   /* Inky */
    { 255, 184, 82, 255 },  /* Clyde */
};

static const Color TEXT_COLORS[TEXT_STYLE_COUNT] = {
    [TEXT_NORMAL]    = { 222, 222, 222, 255 },
    [TEXT_HIGHLIGHT] = { 255, 255, 0, 255 },
    [TEXT_WARNING]   = { 255, 0, 0, 255 },
};

/* ---------- Estado del renderer ---------- */

typedef struct {
    int row;
    TextAlign align;
    TextStyle style;
    char text[MAX_TEXT_LEN];
} TextCommand;

typedef struct {
    int rows;
    int cols;
    Glyph *cells;
    TextCommand texts[MAX_TEXTS];
    int text_count;
} RaylibRenderer;

static Glyph glyph_at(const RaylibRenderer *self, int row, int col)
{
    if (row < 0 || row >= self->rows || col < 0 || col >= self->cols) {
        return GLYPH_EMPTY;
    }
    return self->cells[row * self->cols + col];
}

static bool is_wall(const RaylibRenderer *self, int row, int col)
{
    return glyph_at(self, row, col) == GLYPH_WALL;
}

static Vector2 cell_origin(int row, int col)
{
    return (Vector2){ (float)(col * CELL_SIZE), (float)(row * CELL_SIZE) };
}

static Vector2 cell_center(int row, int col)
{
    const Vector2 o = cell_origin(row, col);
    return (Vector2){ o.x + CELL_SIZE / 2.0f, o.y + CELL_SIZE / 2.0f };
}

static void line(float x1, float y1, float x2, float y2, Color color)
{
    DrawLineEx((Vector2){ x1, y1 }, (Vector2){ x2, y2 }, WALL_THICKNESS, color);
}

/* ---------- Paredes: contorno estilo arcade ---------- */

/* Dibuja los bordes de la pared que dan a un pasillo, recortados según los vecinos. */
static void draw_wall_edges(const RaylibRenderer *self, int r, int c)
{
    const Vector2 o = cell_origin(r, c);
    const float in = WALL_INSET;
    const float x0 = is_wall(self, r, c - 1) ? o.x : o.x + in;
    const float x1 = is_wall(self, r, c + 1) ? o.x + CELL_SIZE : o.x + CELL_SIZE - in;
    const float y0 = is_wall(self, r - 1, c) ? o.y : o.y + in;
    const float y1 = is_wall(self, r + 1, c) ? o.y + CELL_SIZE : o.y + CELL_SIZE - in;

    if (!is_wall(self, r - 1, c)) line(x0, o.y + in, x1, o.y + in, WALL_COLOR);
    if (!is_wall(self, r + 1, c)) line(x0, o.y + CELL_SIZE - in, x1, o.y + CELL_SIZE - in, WALL_COLOR);
    if (!is_wall(self, r, c - 1)) line(o.x + in, y0, o.x + in, y1, WALL_COLOR);
    if (!is_wall(self, r, c + 1)) line(o.x + CELL_SIZE - in, y0, o.x + CELL_SIZE - in, y1, WALL_COLOR);
}

/* Esquinas cóncavas: ambos vecinos son pared pero la diagonal no. */
static void draw_wall_inner_corners(const RaylibRenderer *self, int r, int c)
{
    const Vector2 o = cell_origin(r, c);
    const float in = WALL_INSET;
    const float right = o.x + CELL_SIZE;
    const float bottom = o.y + CELL_SIZE;

    if (is_wall(self, r - 1, c) && is_wall(self, r, c - 1) && !is_wall(self, r - 1, c - 1)) {
        line(o.x, o.y + in, o.x + in, o.y + in, WALL_COLOR);
        line(o.x + in, o.y, o.x + in, o.y + in, WALL_COLOR);
    }
    if (is_wall(self, r - 1, c) && is_wall(self, r, c + 1) && !is_wall(self, r - 1, c + 1)) {
        line(right - in, o.y + in, right, o.y + in, WALL_COLOR);
        line(right - in, o.y, right - in, o.y + in, WALL_COLOR);
    }
    if (is_wall(self, r + 1, c) && is_wall(self, r, c - 1) && !is_wall(self, r + 1, c - 1)) {
        line(o.x, bottom - in, o.x + in, bottom - in, WALL_COLOR);
        line(o.x + in, bottom - in, o.x + in, bottom, WALL_COLOR);
    }
    if (is_wall(self, r + 1, c) && is_wall(self, r, c + 1) && !is_wall(self, r + 1, c + 1)) {
        line(right - in, bottom - in, right, bottom - in, WALL_COLOR);
        line(right - in, bottom - in, right - in, bottom, WALL_COLOR);
    }
}

static void draw_wall(const RaylibRenderer *self, int r, int c)
{
    draw_wall_edges(self, r, c);
    draw_wall_inner_corners(self, r, c);
}

/* ---------- Sprites ---------- */

static void draw_door(int r, int c)
{
    const Vector2 o = cell_origin(r, c);
    DrawRectangleRec((Rectangle){ o.x, o.y + CELL_SIZE * 0.4f, CELL_SIZE, CELL_SIZE * 0.2f }, DOOR_COLOR);
}

static float pacman_mouth_angle(Glyph glyph)
{
    switch (glyph) {
    case GLYPH_PACMAN_DOWN: return 90.0f;
    case GLYPH_PACMAN_LEFT: return 180.0f;
    case GLYPH_PACMAN_UP:   return 270.0f;
    default:                return 0.0f;
    }
}

static void draw_pacman(Vector2 center, Glyph glyph)
{
    const float radius = CELL_SIZE * 0.6f;
    if (glyph == GLYPH_PACMAN_CLOSED) {
        DrawCircleV(center, radius, PACMAN_COLOR);
        return;
    }
    const float mouth = pacman_mouth_angle(glyph);
    DrawCircleSector(center, radius, mouth + 35.0f, mouth + 325.0f, CIRCLE_SEGMENTS, PACMAN_COLOR);
}

static void draw_ghost_body(Vector2 center, float radius, Color body)
{
    const float skirt = radius * 0.8f;
    const float foot = radius / 3.0f;

    DrawCircleV(center, radius, body);
    DrawRectangleV((Vector2){ center.x - radius, center.y }, (Vector2){ 2 * radius, skirt }, body);
    for (int i = 0; i < 3; ++i) {
        const Vector2 foot_center = { center.x - radius + foot + (float)i * 2 * foot, center.y + skirt };
        DrawCircleV(foot_center, foot, body);
    }
}

static void draw_ghost_eyes(Vector2 center, float radius)
{
    for (int side = -1; side <= 1; side += 2) {
        const Vector2 eye = { center.x + (float)side * radius * 0.38f, center.y - radius * 0.15f };
        DrawCircleV(eye, radius * 0.3f, EYE_WHITE);
        DrawCircleV(eye, radius * 0.15f, PUPIL_COLOR);
    }
}

static void draw_scared_face(Vector2 center, float radius, Color face)
{
    const float eye = radius * 0.18f;
    DrawRectangleV((Vector2){ center.x - radius * 0.45f, center.y - radius * 0.35f }, (Vector2){ eye, eye }, face);
    DrawRectangleV((Vector2){ center.x + radius * 0.25f, center.y - radius * 0.35f }, (Vector2){ eye, eye }, face);

    /* Boca en zigzag. */
    const float y = center.y + radius * 0.35f;
    const float step = radius * 0.3f;
    for (int i = 0; i < 5; ++i) {
        const float x = center.x - radius * 0.75f + (float)i * step;
        const float dy = (i % 2 == 0) ? 0.0f : -radius * 0.15f;
        const float ndy = (i % 2 == 0) ? -radius * 0.15f : 0.0f;
        DrawLineEx((Vector2){ x, y + dy }, (Vector2){ x + step, y + ndy }, 1.5f, face);
    }
}

static void draw_ghost(Vector2 center, Glyph glyph)
{
    const float radius = CELL_SIZE * 0.6f;
    center.y -= radius * 0.25f; /* compensa la "pollera" para que quede centrado */

    switch (glyph) {
    case GLYPH_GHOST_EYES:
        draw_ghost_eyes(center, radius);
        break;
    case GLYPH_GHOST_FRIGHTENED:
        draw_ghost_body(center, radius, SCARED_BLUE);
        draw_scared_face(center, radius, SCARED_FACE);
        break;
    case GLYPH_GHOST_FLASHING:
        draw_ghost_body(center, radius, FLASH_WHITE);
        draw_scared_face(center, radius, FLASH_FACE);
        break;
    default:
        draw_ghost_body(center, radius, GHOST_COLORS[glyph - GLYPH_GHOST_BLINKY]);
        draw_ghost_eyes(center, radius);
        break;
    }
}

static void draw_glyph(const RaylibRenderer *self, int r, int c, Glyph glyph)
{
    const Vector2 center = cell_center(r, c);

    switch (glyph) {
    case GLYPH_EMPTY:  break;
    case GLYPH_WALL:   draw_wall(self, r, c); break;
    case GLYPH_DOOR:   draw_door(r, c); break;
    case GLYPH_PELLET: DrawCircleV(center, CELL_SIZE * 0.1f, PELLET_COLOR); break;
    case GLYPH_POWER:  DrawCircleV(center, CELL_SIZE * 0.35f, PELLET_COLOR); break;
    case GLYPH_PACMAN_UP:
    case GLYPH_PACMAN_LEFT:
    case GLYPH_PACMAN_DOWN:
    case GLYPH_PACMAN_RIGHT:
    case GLYPH_PACMAN_CLOSED:
        draw_pacman(center, glyph);
        break;
    default:
        draw_ghost(center, glyph);
        break;
    }
}

static void draw_text_command(const RaylibRenderer *self, const TextCommand *cmd)
{
    const int width = self->cols * CELL_SIZE;
    const int text_width = MeasureText(cmd->text, FONT_SIZE);
    const int y = cmd->row * CELL_SIZE + (CELL_SIZE - FONT_SIZE) / 2;
    int x = TEXT_MARGIN;

    if (cmd->align == ALIGN_CENTER) {
        x = (width - text_width) / 2;
        /* fondo negro para que el texto se lea sobre el laberinto */
        DrawRectangle(x - 4, y - 2, text_width + 8, FONT_SIZE + 4, BACKGROUND);
    } else if (cmd->align == ALIGN_RIGHT) {
        x = width - text_width - TEXT_MARGIN;
    }
    DrawText(cmd->text, x, y, FONT_SIZE, TEXT_COLORS[cmd->style]);
}

/* ---------- Implementación de la interfaz Renderer ---------- */

static void raylib_begin_frame(void *ctx)
{
    RaylibRenderer *self = ctx;
    for (int i = 0; i < self->rows * self->cols; ++i) {
        self->cells[i] = GLYPH_EMPTY;
    }
    self->text_count = 0;
}

static void raylib_draw_cell(void *ctx, int row, int col, Glyph glyph)
{
    RaylibRenderer *self = ctx;
    if (row < 0 || row >= self->rows || col < 0 || col >= self->cols) {
        return;
    }
    if (glyph < 0 || glyph >= GLYPH_COUNT) {
        return;
    }
    self->cells[row * self->cols + col] = glyph;
}

static void raylib_draw_text(void *ctx, int row, TextAlign align, TextStyle style, const char *text)
{
    RaylibRenderer *self = ctx;
    if (self->text_count >= MAX_TEXTS || style < 0 || style >= TEXT_STYLE_COUNT) {
        return;
    }
    TextCommand *cmd = &self->texts[self->text_count++];
    cmd->row = row;
    cmd->align = align;
    cmd->style = style;
    strncpy(cmd->text, text, MAX_TEXT_LEN - 1);
    cmd->text[MAX_TEXT_LEN - 1] = '\0';
}

static void raylib_end_frame(void *ctx)
{
    const RaylibRenderer *self = ctx;

    BeginDrawing();
    ClearBackground(BACKGROUND);
    for (int r = 0; r < self->rows; ++r) {
        for (int c = 0; c < self->cols; ++c) {
            draw_glyph(self, r, c, glyph_at(self, r, c));
        }
    }
    for (int i = 0; i < self->text_count; ++i) {
        draw_text_command(self, &self->texts[i]);
    }
    EndDrawing();
}

static void raylib_destroy(void *ctx)
{
    RaylibRenderer *self = ctx;
    CloseWindow();
    free(self->cells);
    free(self);
}

/* ---------- FACTORY ---------- */

bool raylib_renderer_create(Renderer *out, int rows, int cols)
{
    RaylibRenderer *self = calloc(1, sizeof *self);
    if (self == NULL) {
        return false;
    }
    self->rows = rows;
    self->cols = cols;
    self->cells = calloc((size_t)(rows * cols), sizeof *self->cells);
    if (self->cells == NULL) {
        free(self);
        return false;
    }

    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(cols * CELL_SIZE, rows * CELL_SIZE, "Pac-Man");
    if (!IsWindowReady()) {
        free(self->cells);
        free(self);
        return false;
    }

    out->ctx = self;
    out->begin_frame = raylib_begin_frame;
    out->draw_cell = raylib_draw_cell;
    out->draw_text = raylib_draw_text;
    out->end_frame = raylib_end_frame;
    out->destroy = raylib_destroy;
    return true;
}
