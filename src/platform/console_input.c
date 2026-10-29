#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include "platform/console_input.h"

#include <stdlib.h>

/* Mapeo común de teclas "normales" a órdenes del juego. */
static Command command_from_key(int key)
{
    switch (key) {
    case 'w': case 'W': return CMD_UP;
    case 's': case 'S': return CMD_DOWN;
    case 'a': case 'A': return CMD_LEFT;
    case 'd': case 'D': return CMD_RIGHT;
    case 'p': case 'P': case ' ': return CMD_PAUSE;
    case 'r': case 'R': case '\r': case '\n': return CMD_RESTART;
    case 'q': case 'Q': return CMD_QUIT;
    default: return CMD_NONE;
    }
}

#ifdef _WIN32
/* ======================= Windows (conio) ======================= */

#include <conio.h>

#define KEY_EXTENDED_A 0
#define KEY_EXTENDED_B 224
#define KEY_ESCAPE 27

static Command command_from_arrow(int code)
{
    switch (code) {
    case 72: return CMD_UP;
    case 80: return CMD_DOWN;
    case 75: return CMD_LEFT;
    case 77: return CMD_RIGHT;
    default: return CMD_NONE;
    }
}

static Command console_poll(void *ctx)
{
    (void)ctx;
    while (_kbhit()) {
        const int key = _getch();
        Command cmd;
        if (key == KEY_EXTENDED_A || key == KEY_EXTENDED_B) {
            cmd = command_from_arrow(_getch());
        } else if (key == KEY_ESCAPE) {
            cmd = CMD_QUIT;
        } else {
            cmd = command_from_key(key);
        }
        if (cmd != CMD_NONE) {
            return cmd;
        }
    }
    return CMD_NONE;
}

static void console_destroy(void *ctx)
{
    (void)ctx;
}

bool console_input_create(InputSource *out)
{
    out->ctx = NULL;
    out->poll = console_poll;
    out->destroy = console_destroy;
    return true;
}

#else
/* ======================= POSIX (termios) ======================= */

#include <termios.h>
#include <unistd.h>

typedef struct {
    struct termios original;
} PosixInput;

static int read_byte(void)
{
    unsigned char c;
    return read(STDIN_FILENO, &c, 1) == 1 ? c : -1;
}

static Command command_from_escape(void)
{
    if (read_byte() != '[') {
        return CMD_QUIT; /* ESC sola */
    }
    switch (read_byte()) {
    case 'A': return CMD_UP;
    case 'B': return CMD_DOWN;
    case 'C': return CMD_RIGHT;
    case 'D': return CMD_LEFT;
    default:  return CMD_NONE;
    }
}

static Command console_poll(void *ctx)
{
    (void)ctx;
    int key;
    while ((key = read_byte()) != -1) {
        const Command cmd = (key == 27) ? command_from_escape() : command_from_key(key);
        if (cmd != CMD_NONE) {
            return cmd;
        }
    }
    return CMD_NONE;
}

static void console_destroy(void *ctx)
{
    PosixInput *self = ctx;
    tcsetattr(STDIN_FILENO, TCSANOW, &self->original);
    free(self);
}

bool console_input_create(InputSource *out)
{
    PosixInput *self = malloc(sizeof *self);
    if (self == NULL || tcgetattr(STDIN_FILENO, &self->original) != 0) {
        free(self);
        return false;
    }
    struct termios raw = self->original;
    raw.c_lflag &= (tcflag_t)~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    out->ctx = self;
    out->poll = console_poll;
    out->destroy = console_destroy;
    return true;
}

#endif
