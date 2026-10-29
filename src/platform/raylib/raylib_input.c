#include "platform/raylib_input.h"

#include <stdlib.h>

#include "raylib.h"

typedef struct {
    bool quit_sent;
} RaylibInput;

static Command command_from_key(int key)
{
    switch (key) {
    case KEY_UP:    case KEY_W: return CMD_UP;
    case KEY_DOWN:  case KEY_S: return CMD_DOWN;
    case KEY_LEFT:  case KEY_A: return CMD_LEFT;
    case KEY_RIGHT: case KEY_D: return CMD_RIGHT;
    case KEY_P:     case KEY_SPACE: return CMD_PAUSE;
    case KEY_R:     case KEY_ENTER: case KEY_KP_ENTER: return CMD_RESTART;
    case KEY_Q:     return CMD_QUIT;
    default:        return CMD_NONE;
    }
}

static Command raylib_poll(void *ctx)
{
    RaylibInput *self = ctx;

    /* Cerrar la ventana (X o Esc) se reporta una sola vez. */
    if (WindowShouldClose() && !self->quit_sent) {
        self->quit_sent = true;
        return CMD_QUIT;
    }
    int key;
    while ((key = GetKeyPressed()) != 0) {
        const Command command = command_from_key(key);
        if (command != CMD_NONE) {
            return command;
        }
    }
    return CMD_NONE;
}

static void raylib_input_destroy(void *ctx)
{
    free(ctx);
}

bool raylib_input_create(InputSource *out)
{
    RaylibInput *self = calloc(1, sizeof *self);
    if (self == NULL || !IsWindowReady()) {
        free(self);
        return false;
    }
    out->ctx = self;
    out->poll = raylib_poll;
    out->destroy = raylib_input_destroy;
    return true;
}
