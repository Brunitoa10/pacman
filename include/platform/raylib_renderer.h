#ifndef PACMAN_PLATFORM_RAYLIB_RENDERER_H
#define PACMAN_PLATFORM_RAYLIB_RENDERER_H

#include <stdbool.h>

#include "ports/renderer.h"

/*
 * Renderer gráfico con raylib. Abre la ventana al crearse y la cierra al
 * destruirse. Guarda el cuadro en un buffer de glifos y lo dibuja en
 * end_frame, lo que le permite dibujar las paredes "conectadas" mirando
 * a sus vecinas (algo que la vista no necesita saber).
 */
bool raylib_renderer_create(Renderer *out, int rows, int cols);

#endif /* PACMAN_PLATFORM_RAYLIB_RENDERER_H */
