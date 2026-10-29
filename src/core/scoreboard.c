#include "core/scoreboard.h"

#include "core/config.h"

void scoreboard_init(Scoreboard *scoreboard, const EventBus *bus, int high_score)
{
    scoreboard->bus = bus;
    scoreboard->high_score = high_score;
    scoreboard_reset(scoreboard);
}

void scoreboard_reset(Scoreboard *scoreboard)
{
    scoreboard->score = 0;
    scoreboard->next_extra_life = EXTRA_LIFE_SCORE;
}

static int points_for(const GameEvent *event)
{
    switch (event->type) {
    case EVENT_PELLET_EATEN: return SCORE_PELLET;
    case EVENT_POWER_EATEN:  return SCORE_POWER;
    case EVENT_GHOST_EATEN:  return event->value;
    default:                 return 0;
    }
}

static void add_points(Scoreboard *scoreboard, int points)
{
    scoreboard->score += points;
    if (scoreboard->score > scoreboard->high_score) {
        scoreboard->high_score = scoreboard->score;
    }
    if (scoreboard->score >= scoreboard->next_extra_life) {
        scoreboard->next_extra_life += EXTRA_LIFE_SCORE;
        event_bus_publish(scoreboard->bus, EVENT_EXTRA_LIFE, 1);
    }
}

void scoreboard_on_event(void *listener, const GameEvent *event)
{
    const int points = points_for(event);
    if (points > 0) {
        add_points((Scoreboard *)listener, points);
    }
}
