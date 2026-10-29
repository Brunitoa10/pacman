#ifndef PACMAN_CORE_SCOREBOARD_H
#define PACMAN_CORE_SCOREBOARD_H

#include "core/event_bus.h"

/*
 * Observador del EventBus: traduce eventos del juego en puntos.
 * Única responsabilidad: el puntaje (y avisar cuando se gana una vida extra).
 */
typedef struct {
    int score;
    int high_score;
    int next_extra_life;
    const EventBus *bus; /* para publicar EVENT_EXTRA_LIFE */
} Scoreboard;

void scoreboard_init(Scoreboard *scoreboard, const EventBus *bus, int high_score);
void scoreboard_reset(Scoreboard *scoreboard);

/* Handler compatible con EventHandler; se registra en el bus. */
void scoreboard_on_event(void *listener, const GameEvent *event);

#endif /* PACMAN_CORE_SCOREBOARD_H */
