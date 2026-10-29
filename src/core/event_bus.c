#include "core/event_bus.h"

#include <stddef.h>

void event_bus_init(EventBus *bus)
{
    bus->count = 0;
}

bool event_bus_subscribe(EventBus *bus, EventHandler handler, void *listener)
{
    if (handler == NULL || bus->count >= EVENT_BUS_MAX_SUBSCRIBERS) {
        return false;
    }
    bus->subscribers[bus->count].handler = handler;
    bus->subscribers[bus->count].listener = listener;
    bus->count++;
    return true;
}

void event_bus_publish(const EventBus *bus, GameEventType type, int value)
{
    const GameEvent event = { type, value };
    for (int i = 0; i < bus->count; ++i) {
        bus->subscribers[i].handler(bus->subscribers[i].listener, &event);
    }
}
