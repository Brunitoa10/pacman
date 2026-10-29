#include "core/entity.h"

void entity_place(Entity *entity, Vec2 pos, Direction dir, int move_interval)
{
    entity->pos = pos;
    entity->dir = dir;
    entity->move_interval = move_interval;
    entity->move_counter = 0;
}

void entity_set_interval(Entity *entity, int move_interval)
{
    entity->move_interval = move_interval > 0 ? move_interval : 1;
}

bool entity_tick(Entity *entity)
{
    if (++entity->move_counter < entity->move_interval) {
        return false;
    }
    entity->move_counter = 0;
    return true;
}

void entity_move_to(Entity *entity, Vec2 pos, Direction dir)
{
    entity->pos = pos;
    entity->dir = dir;
}
