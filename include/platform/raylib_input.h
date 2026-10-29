#ifndef PACMAN_PLATFORM_RAYLIB_INPUT_H
#define PACMAN_PLATFORM_RAYLIB_INPUT_H

#include <stdbool.h>

#include "ports/input.h"

/* Teclado de la ventana raylib. Requiere que la ventana ya esté abierta. */
bool raylib_input_create(InputSource *out);

#endif /* PACMAN_PLATFORM_RAYLIB_INPUT_H */
