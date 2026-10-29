#ifndef PACMAN_CORE_RNG_H
#define PACMAN_CORE_RNG_H

#include <stdint.h>

/*
 * Generador pseudoaleatorio propio (xorshift32). Se inyecta donde hace falta
 * en lugar de usar rand() global: el juego es reproducible y testeable.
 */
typedef struct {
    uint32_t state;
} Rng;

void rng_seed(Rng *rng, uint32_t seed);
uint32_t rng_next(Rng *rng);
int rng_range(Rng *rng, int upper_exclusive);

#endif /* PACMAN_CORE_RNG_H */
