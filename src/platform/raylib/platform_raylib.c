#include "platform/platform.h"

#include "platform/raylib_input.h"
#include "platform/raylib_renderer.h"

bool platform_create(Platform *out, int rows, int cols)
{
    /* El orden importa: la entrada de raylib necesita la ventana abierta. */
    if (!raylib_renderer_create(&out->renderer, rows, cols)) {
        return false;
    }
    if (!raylib_input_create(&out->input)) {
        out->renderer.destroy(out->renderer.ctx);
        return false;
    }
    return true;
}

void platform_destroy(Platform *platform)
{
    platform->input.destroy(platform->input.ctx);
    platform->renderer.destroy(platform->renderer.ctx);
}
