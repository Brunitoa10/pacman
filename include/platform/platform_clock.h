#ifndef PACMAN_PLATFORM_CLOCK_H
#define PACMAN_PLATFORM_CLOCK_H

#include <stdint.h>

uint64_t platform_clock_now_ms(void);
void platform_clock_sleep_ms(uint32_t ms);

#endif /* PACMAN_PLATFORM_CLOCK_H */
