#ifndef PACMAN_PORTS_INPUT_H
#define PACMAN_PORTS_INPUT_H

#include "core/command.h"

/*
 * Puerto de entrada: cualquier fuente de órdenes (teclado, joystick, bot,
 * repetición grabada) implementa esta interfaz.
 */
typedef struct InputSource {
    void *ctx;
    Command (*poll)(void *ctx); /* CMD_NONE cuando no hay más entradas pendientes */
    void (*destroy)(void *ctx);
} InputSource;

#endif /* PACMAN_PORTS_INPUT_H */
