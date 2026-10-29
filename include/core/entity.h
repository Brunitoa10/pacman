#ifndef PACMAN_CORE_ENTITY_H
#define PACMAN_CORE_ENTITY_H

#include <stdbool.h>

#include "core/direction.h"
#include "core/vec2.h"

/*
 * Parte común a todo lo que se mueve por la grilla (composición en lugar de
 * herencia: Pacman y Ghost "tienen un" Entity).
 */
typedef struct {
    Vec2 pos;
    Direction dir;
    int move_interval; /* ticks por casilla */
    int move_counter;
} Entity;

void entity_place(Entity *entity, Vec2 pos, Direction dir, int move_interval);
void entity_set_interval(Entity *entity, int move_interval);

/* Avanza el reloj interno; devuelve true cuando toca moverse una casilla. */
bool entity_tick(Entity *entity);

void entity_move_to(Entity *entity, Vec2 pos, Direction dir);

#endif /* PACMAN_CORE_ENTITY_H */
