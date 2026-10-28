#include "loading_overlay_painter.h"

#include "../constants.h"
#include "../functions.h"
#include "painter.h"

float g_animation_state = 0.0f;
uint64_t g_last_paint_time = -1;

void paintLoadingOverlay(SDL_Renderer *renderer) {
    if (g_last_paint_time == -1) {
        g_last_paint_time = SDL_GetTicks();
    }
    const uint64_t time_elapsed = SDL_GetTicks() - g_last_paint_time;
    g_animation_state += (float) time_elapsed / 6.0f;
    while (g_animation_state > 200.0f) {
        g_animation_state -= 200.0f;
    }

    float radius1;
    if (g_animation_state >= 100.0f) {
        radius1 = 200.0f - g_animation_state;
    } else {
        radius1 = g_animation_state;
    }
    const Circle circle1 = {
        .x = WINDOW_WIDTH / 2.0f,
        .y = WINDOW_HEIGHT / 2.0f,
        .outer_radius = radius1 + 15,
        .inner_radius = radius1,
    };
    const int color1 = max(0, min(255, (int) (radius1 / 100.0f * 255.0f)));
    paintFilledCircle(renderer, circle1, (SDL_Color) {.r = color1, .g = color1, .b = color1, .a = 255});

    const float radius2 = 100.0f - radius1;
    const Circle circle2 = {
        .x = WINDOW_WIDTH / 2.0f,
        .y = WINDOW_HEIGHT / 2.0f,
        .outer_radius = radius2 + 15,
        .inner_radius = radius2,
    };
    const int color2 = max(0, min(255, (int) (radius2 / 100.0f * 255.0f)));
    paintFilledCircle(renderer, circle2, (SDL_Color) {.r = color2, .g = color2, .b = color2, .a = 255});

    char* text;
    if (g_animation_state <= 66.67f) {
        text = "Loading.";
    } else if (g_animation_state <= 133.33f) {
        text = "Loading..";
    } else {
        text = "Loading...";
    }

    const SDL_FPoint pt = {.x = WINDOW_WIDTH / 2.0f - 120.0f, .y = WINDOW_HEIGHT / 2.0f + 150.0f};
    const SDL_Color text_color = {.r = 255, .g = 255, .b = 255, .a = 255};
    paintText(renderer, text, pt, 60.0f, text_color, FONT_PIXELOID);

    SDL_SetRenderDrawColor(renderer, 38, 31, 28, 100);
    SDL_RenderFillRect(renderer, &(SDL_FRect) {.x = 0, .y = 0, .w = WINDOW_WIDTH, .h = WINDOW_HEIGHT});

    g_last_paint_time = SDL_GetTicks();
}

void resetLoadingOverlay() {
    g_animation_state = 0.0f;
    g_last_paint_time = -1;
}