#ifndef PACMAN_CORE_MODE_SCHEDULE_H
#define PACMAN_CORE_MODE_SCHEDULE_H

#include <stdbool.h>

/* Modo global de los fantasmas: dispersarse a su esquina o perseguir. */
typedef enum {
    MODE_SCATTER,
    MODE_CHASE
} GhostMode;

/* Responsabilidad única: llevar el cronograma dispersión/persecución. */
typedef struct {
    int phase;
    int ticks_left;
} ModeSchedule;

void mode_schedule_reset(ModeSchedule *schedule);

/* Devuelve true si en este tick cambió el modo. */
bool mode_schedule_tick(ModeSchedule *schedule);

GhostMode mode_schedule_current(const ModeSchedule *schedule);

#endif /* PACMAN_CORE_MODE_SCHEDULE_H */
