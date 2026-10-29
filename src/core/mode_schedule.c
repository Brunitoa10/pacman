#include "core/mode_schedule.h"

#include "core/config.h"

/* Fases pares = dispersión, impares = persecución. Tras la última: persecución infinita. */
static const int PHASE_SECONDS[] = { 7, 20, 7, 20, 5, 20, 5 };
static const int PHASE_COUNT = (int)(sizeof PHASE_SECONDS / sizeof PHASE_SECONDS[0]);

static void enter_phase(ModeSchedule *schedule, int phase)
{
    schedule->phase = phase;
    schedule->ticks_left = phase < PHASE_COUNT ? SECONDS_TO_TICKS(PHASE_SECONDS[phase]) : 0;
}

void mode_schedule_reset(ModeSchedule *schedule)
{
    enter_phase(schedule, 0);
}

bool mode_schedule_tick(ModeSchedule *schedule)
{
    if (schedule->phase >= PHASE_COUNT) {
        return false;
    }
    if (--schedule->ticks_left > 0) {
        return false;
    }
    enter_phase(schedule, schedule->phase + 1);
    return true;
}

GhostMode mode_schedule_current(const ModeSchedule *schedule)
{
    return (schedule->phase % 2 == 0) ? MODE_SCATTER : MODE_CHASE;
}
