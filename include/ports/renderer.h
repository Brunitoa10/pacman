#ifndef PACMAN_PORTS_RENDERER_H
#define PACMAN_PORTS_RENDERER_H

/*
 * PATRÓN BRIDGE / INTERFAZ (puerto de salida).
 * La vista dibuja "qué" (glifos semánticos); cada implementación decide "cómo"
 * (consola ANSI, SDL, raylib...). Cambiar de biblioteca gráfica no toca el juego.
 */
typedef enum {
    GLYPH_EMPTY,
    GLYPH_WALL,
    GLYPH_DOOR,
    GLYPH_PELLET,
    GLYPH_POWER,
    GLYPH_PACMAN_UP,
    GLYPH_PACMAN_LEFT,
    GLYPH_PACMAN_DOWN,
    GLYPH_PACMAN_RIGHT,
    GLYPH_PACMAN_CLOSED,
    GLYPH_GHOST_BLINKY,
    GLYPH_GHOST_PINKY,
    GLYPH_GHOST_INKY,
    GLYPH_GHOST_CLYDE,
    GLYPH_GHOST_FRIGHTENED,
    GLYPH_GHOST_FLASHING,
    GLYPH_GHOST_EYES,
    GLYPH_COUNT
} Glyph;

typedef enum {
    TEXT_NORMAL,
    TEXT_HIGHLIGHT,
    TEXT_WARNING,
    TEXT_STYLE_COUNT
} TextStyle;

typedef enum {
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT
} TextAlign;

typedef struct Renderer {
    void *ctx;
    void (*begin_frame)(void *ctx);
    void (*draw_cell)(void *ctx, int row, int col, Glyph glyph);
    void (*draw_text)(void *ctx, int row, TextAlign align, TextStyle style, const char *text);
    void (*end_frame)(void *ctx);
    void (*destroy)(void *ctx);
} Renderer;

#endif /* PACMAN_PORTS_RENDERER_H */
