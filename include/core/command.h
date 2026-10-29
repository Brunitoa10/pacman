#ifndef PACMAN_CORE_COMMAND_H
#define PACMAN_CORE_COMMAND_H

/*
 * Órdenes del jugador, independientes del dispositivo de entrada.
 * El teclado, un joystick o un bot de pruebas producen Commands; el juego
 * sólo conoce esta abstracción (Inversión de Dependencias).
 */
typedef enum {
    CMD_NONE,
    CMD_UP,
    CMD_DOWN,
    CMD_LEFT,
    CMD_RIGHT,
    CMD_PAUSE,
    CMD_RESTART,
    CMD_QUIT,
    CMD_COUNT
} Command;

#endif /* PACMAN_CORE_COMMAND_H */
