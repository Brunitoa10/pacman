#include "platform/platform.h"

#include "platform/console_input.h"
#include "platform/console_renderer.h"

bool platform_create(Platform *out, int rows, int cols)
{
    if (!console_renderer_create(&out->renderer, rows, cols)) {
        return false;
    }
    if (!console_input_create(&out->input)) {
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
