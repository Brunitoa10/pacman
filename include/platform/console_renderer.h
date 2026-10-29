#ifndef PACMAN_PLATFORM_CONSOLE_RENDERER_H
#define PACMAN_PLATFORM_CONSOLE_RENDERER_H

#include <stdbool.h>

#include "ports/renderer.h"

/*
 * Implementación del Renderer para terminal con secuencias ANSI
 * (Windows 10+ y Linux/macOS). Usa doble buffer: arma el cuadro completo en
 * memoria y lo escribe de una sola vez para evitar parpadeo.
 */
bool console_renderer_create(Renderer *out, int rows, int cols);

#endif /* PACMAN_PLATFORM_CONSOLE_RENDERER_H */
