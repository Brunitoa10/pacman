#include "platform/console_renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

#define CHARS_PER_CELL 2

typedef enum {
    CC_DEFAULT,
    CC_WALL,
    CC_DOOR,
    CC_PELLET,
    CC_PACMAN,
    CC_RED,
    CC_PINK,
    CC_CYAN,
    CC_ORANGE,
    CC_FRIGHTENED,
    CC_WHITE,
    CC_TEXT,
    CC_HIGHLIGHT,
    CC_WARNING,
    CC_COUNT
} ConsoleColor;

static const char *const CC_CODES[CC_COUNT] = {
    [CC_DEFAULT]    = "\x1b[0m",
    [CC_WALL]       = "\x1b[0;48;5;19m",
    [CC_DOOR]       = "\x1b[0;38;5;213m",
    [CC_PELLET]     = "\x1b[0;38;5;223m",
    [CC_PACMAN]     = "\x1b[0;1;38;5;226m",
    [CC_RED]        = "\x1b[0;1;38;5;196m",
    [CC_PINK]       = "\x1b[0;1;38;5;213m",
    [CC_CYAN]       = "\x1b[0;1;38;5;51m",
    [CC_ORANGE]     = "\x1b[0;1;38;5;208m",
    [CC_FRIGHTENED] = "\x1b[0;1;38;5;27m",
    [CC_WHITE]      = "\x1b[0;1;38;5;231m",
    [CC_TEXT]       = "\x1b[0;38;5;252m",
    [CC_HIGHLIGHT]  = "\x1b[0;1;38;5;226m",
    [CC_WARNING]    = "\x1b[0;1;38;5;196m",
};

/* Cómo se ve cada glifo semántico en la terminal (2 caracteres por casilla). */
typedef struct {
    char text[CHARS_PER_CELL + 1];
    ConsoleColor color;
} GlyphStyle;

static const GlyphStyle GLYPH_STYLES[GLYPH_COUNT] = {
    [GLYPH_EMPTY]            = { "  ", CC_DEFAULT },
    [GLYPH_WALL]             = { "  ", CC_WALL },
    [GLYPH_DOOR]             = { "--", CC_DOOR },
    [GLYPH_PELLET]           = { ". ", CC_PELLET },
    [GLYPH_POWER]            = { "o ", CC_PELLET },
    [GLYPH_PACMAN_UP]        = { "V ", CC_PACMAN },
    [GLYPH_PACMAN_LEFT]      = { "> ", CC_PACMAN },
    [GLYPH_PACMAN_DOWN]      = { "^ ", CC_PACMAN },
    [GLYPH_PACMAN_RIGHT]     = { "< ", CC_PACMAN },
    [GLYPH_PACMAN_CLOSED]    = { "O ", CC_PACMAN },
    [GLYPH_GHOST_BLINKY]     = { "M ", CC_RED },
    [GLYPH_GHOST_PINKY]      = { "M ", CC_PINK },
    [GLYPH_GHOST_INKY]       = { "M ", CC_CYAN },
    [GLYPH_GHOST_CLYDE]      = { "M ", CC_ORANGE },
    [GLYPH_GHOST_FRIGHTENED] = { "W ", CC_FRIGHTENED },
    [GLYPH_GHOST_FLASHING]   = { "W ", CC_WHITE },
    [GLYPH_GHOST_EYES]       = { "\" ", CC_WHITE },
};

static const ConsoleColor TEXT_COLORS[TEXT_STYLE_COUNT] = {
    [TEXT_NORMAL]    = CC_TEXT,
    [TEXT_HIGHLIGHT] = CC_HIGHLIGHT,
    [TEXT_WARNING]   = CC_WARNING,
};

typedef struct {
    char ch;
    unsigned char color;
} ConsoleChar;

typedef struct {
    int rows;
    int width; /* en caracteres */
    ConsoleChar *buffer;
    char *output;
    size_t output_capacity;
#ifdef _WIN32
    HANDLE handle;
    DWORD original_mode;
    bool mode_saved;
#endif
} ConsoleRenderer;

/* ---------- helpers ---------- */

static ConsoleChar *char_at(ConsoleRenderer *self, int row, int x)
{
    return &self->buffer[row * self->width + x];
}

static void put_char(ConsoleRenderer *self, int row, int x, char ch, ConsoleColor color)
{
    if (row < 0 || row >= self->rows || x < 0 || x >= self->width) {
        return;
    }
    ConsoleChar *cell = char_at(self, row, x);
    cell->ch = ch;
    cell->color = (unsigned char)color;
}

static size_t longest_color_code(void)
{
    size_t longest = 0;
    for (int i = 0; i < CC_COUNT; ++i) {
        const size_t len = strlen(CC_CODES[i]);
        if (len > longest) {
            longest = len;
        }
    }
    return longest;
}

static void enable_ansi(ConsoleRenderer *self)
{
#ifdef _WIN32
    DWORD mode = 0;
    self->handle = GetStdHandle(STD_OUTPUT_HANDLE);
    self->mode_saved = GetConsoleMode(self->handle, &mode) != 0;
    if (self->mode_saved) {
        self->original_mode = mode;
        SetConsoleMode(self->handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#else
    (void)self;
#endif
}

static void restore_console(ConsoleRenderer *self)
{
#ifdef _WIN32
    if (self->mode_saved) {
        SetConsoleMode(self->handle, self->original_mode);
    }
#else
    (void)self;
#endif
}

/* ---------- implementación de la interfaz Renderer ---------- */

static void console_begin_frame(void *ctx)
{
    ConsoleRenderer *self = ctx;
    for (int i = 0; i < self->rows * self->width; ++i) {
        self->buffer[i].ch = ' ';
        self->buffer[i].color = CC_DEFAULT;
    }
}

static void console_draw_cell(void *ctx, int row, int col, Glyph glyph)
{
    ConsoleRenderer *self = ctx;
    if (glyph < 0 || glyph >= GLYPH_COUNT) {
        return;
    }
    const GlyphStyle *style = &GLYPH_STYLES[glyph];
    for (int i = 0; i < CHARS_PER_CELL; ++i) {
        put_char(self, row, col * CHARS_PER_CELL + i, style->text[i], style->color);
    }
}

static int aligned_start(int width, int text_len, TextAlign align)
{
    switch (align) {
    case ALIGN_CENTER: return (width - text_len) / 2;
    case ALIGN_RIGHT:  return width - text_len;
    default:           return 0;
    }
}

static void console_draw_text(void *ctx, int row, TextAlign align, TextStyle style, const char *text)
{
    ConsoleRenderer *self = ctx;
    const int len = (int)strlen(text);
    const int start = aligned_start(self->width, len, align);
    const ConsoleColor color = TEXT_COLORS[style];

    for (int i = 0; i < len; ++i) {
        put_char(self, row, start + i, text[i], color);
    }
}

static size_t append(char *dst, size_t pos, const char *src)
{
    const size_t len = strlen(src);
    memcpy(dst + pos, src, len);
    return pos + len;
}

static void console_end_frame(void *ctx)
{
    ConsoleRenderer *self = ctx;
    size_t pos = append(self->output, 0, "\x1b[H");

    for (int row = 0; row < self->rows; ++row) {
        int current = -1;
        for (int x = 0; x < self->width; ++x) {
            const ConsoleChar *cell = char_at(self, row, x);
            if (cell->color != current) {
                current = cell->color;
                pos = append(self->output, pos, CC_CODES[current]);
            }
            self->output[pos++] = cell->ch;
        }
        pos = append(self->output, pos, CC_CODES[CC_DEFAULT]);
        if (row < self->rows - 1) {
            self->output[pos++] = '\n';
        }
    }
    fwrite(self->output, 1, pos, stdout);
    fflush(stdout);
}

static void console_destroy(void *ctx)
{
    ConsoleRenderer *self = ctx;
    fputs("\x1b[0m\x1b[2J\x1b[H\x1b[?25h", stdout);
    fflush(stdout);
    restore_console(self);
    free(self->buffer);
    free(self->output);
    free(self);
}

/* ---------- FACTORY ---------- */

bool console_renderer_create(Renderer *out, int rows, int cols)
{
    ConsoleRenderer *self = calloc(1, sizeof *self);
    if (self == NULL) {
        return false;
    }
    self->rows = rows;
    self->width = cols * CHARS_PER_CELL;
    self->buffer = calloc((size_t)(rows * self->width), sizeof *self->buffer);
    /* Peor caso: un código de color por carácter + reset y salto por fila. */
    self->output_capacity = (size_t)rows * ((size_t)self->width * (longest_color_code() + 1)
                          + longest_color_code() + 2) + 16;
    self->output = malloc(self->output_capacity);
    if (self->buffer == NULL || self->output == NULL) {
        free(self->buffer);
        free(self->output);
        free(self);
        return false;
    }

    enable_ansi(self);
    /* Intenta ajustar el tamaño de la ventana, oculta el cursor y limpia. */
    printf("\x1b[8;%d;%dt\x1b[?25l\x1b[2J", rows, self->width);
    fflush(stdout);

    out->ctx = self;
    out->begin_frame = console_begin_frame;
    out->draw_cell = console_draw_cell;
    out->draw_text = console_draw_text;
    out->end_frame = console_end_frame;
    out->destroy = console_destroy;
    return true;
}
