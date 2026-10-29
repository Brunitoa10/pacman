#ifndef PACMAN_CORE_EVENT_BUS_H
#define PACMAN_CORE_EVENT_BUS_H

#include <stdbool.h>

/*
 * PATRÓN OBSERVER: el juego publica "qué pasó" sin saber quién escucha.
 * Puntaje, sonido, logros o estadísticas se suscriben sin modificar game.c.
 */
typedef enum {
    EVENT_PELLET_EATEN,
    EVENT_POWER_EATEN,
    EVENT_GHOST_EATEN,   /* value = puntos otorgados */
    EVENT_PACMAN_DIED,
    EVENT_LEVEL_CLEARED,
    EVENT_EXTRA_LIFE,
    EVENT_TYPE_COUNT
} GameEventType;

typedef struct {
    GameEventType type;
    int value;
} GameEvent;

typedef void (*EventHandler)(void *listener, const GameEvent *event);

#define EVENT_BUS_MAX_SUBSCRIBERS 8

typedef struct {
    EventHandler handler;
    void *listener;
} Subscription;

typedef struct {
    Subscription subscribers[EVENT_BUS_MAX_SUBSCRIBERS];
    int count;
} EventBus;

void event_bus_init(EventBus *bus);
bool event_bus_subscribe(EventBus *bus, EventHandler handler, void *listener);
void event_bus_publish(const EventBus *bus, GameEventType type, int value);

#endif /* PACMAN_CORE_EVENT_BUS_H */
