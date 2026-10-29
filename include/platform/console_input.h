#ifndef PACMAN_PLATFORM_CONSOLE_INPUT_H
#define PACMAN_PLATFORM_CONSOLE_INPUT_H

#include <stdbool.h>

#include "ports/input.h"

/* Implementación de InputSource con teclado de consola, no bloqueante. */
bool console_input_create(InputSource *out);

#endif /* PACMAN_PLATFORM_CONSOLE_INPUT_H */
