#ifndef PACMAN_VIEW_GAME_VIEW_H
#define PACMAN_VIEW_GAME_VIEW_H

#include "core/game.h"
#include "ports/renderer.h"

/* Tamaño de la pantalla lógica que necesita la vista (en casillas). */
int game_view_rows(void);
int game_view_cols(void);

/* Traduce el estado del juego en llamadas al Renderer. No modifica el juego. */
void game_view_render(const Game *game, const Renderer *renderer);

#endif /* PACMAN_VIEW_GAME_VIEW_H */
