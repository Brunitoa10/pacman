/*
 * Composition root: arma las dependencias, las inyecta y corre el bucle.
 * No sabe si dibuja en consola o en una ventana: eso lo decide qué
 * implementación de platform_create() se compila (Abstract Factory).
 *   build.bat         -> consola
 *   build_raylib.bat  -> ventana gráfica con raylib
 */
#include <stdio.h>

#include "core/config.h"
#include "core/game.h"
#include "platform/platform.h"
#include "platform/platform_clock.h"
#include "view/game_view.h"

static void process_input(Game *game, const InputSource *input)
{
    Command command;
    while ((command = input->poll(input->ctx)) != CMD_NONE) {
        game_handle_command(game, command);
    }
}

/* Bucle de paso fijo: la simulación corre igual en cualquier PC. */
static void run(Game *game, const Platform *platform)
{
    uint64_t next_tick = platform_clock_now_ms();

    while (!game_should_quit(game)) {
        process_input(game, &platform->input);
        game_tick(game);
        game_view_render(game, &platform->renderer);

        next_tick += TICK_MS;
        const uint64_t now = platform_clock_now_ms();
        if (next_tick > now) {
            platform_clock_sleep_ms((uint32_t)(next_tick - now));
        } else {
            next_tick = now; /* vamos atrasados: no acumular deuda */
        }
    }
}

int main(void)
{
    Platform platform;
    static Game game; /* grande: fuera del stack */

    if (!platform_create(&platform, game_view_rows(), game_view_cols())) {
        fputs("No se pudo inicializar la plataforma (pantalla/teclado).\n", stderr);
        return 1;
    }

    game_init(&game, (uint32_t)platform_clock_now_ms());
    run(&game, &platform);

    platform_destroy(&platform);
    printf("Gracias por jugar. Puntaje final: %d\n", game.scoreboard.score);
    return 0;
}
