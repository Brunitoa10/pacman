#ifndef PACMAN_PLATFORM_PLATFORM_H
#define PACMAN_PLATFORM_PLATFORM_H

#include <stdbool.h>

#include "ports/input.h"
#include "ports/renderer.h"

/*
 * PATRÓN ABSTRACT FACTORY: crea una "familia" coherente de implementaciones
 * (renderer + entrada) para un backend. Hay una implementación por backend:
 *   - src/platform/platform_console.c  -> terminal ANSI
 *   - src/platform/raylib/platform_raylib.c -> ventana gráfica con raylib
 * El que se compila decide el backend; main.c no cambia.
 */
typedef struct {
    Renderer renderer;
    InputSource input;
} Platform;

bool platform_create(Platform *out, int rows, int cols);
void platform_destroy(Platform *platform);

#endif /* PACMAN_PLATFORM_PLATFORM_H */
