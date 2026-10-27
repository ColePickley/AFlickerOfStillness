#ifndef AFLICKEROFSTILLNESS_LOADING_OVERLAY_PAINTER_H
#define AFLICKEROFSTILLNESS_LOADING_OVERLAY_PAINTER_H
#include "SDL3/SDL_render.h"

extern float g_animation_state;
extern uint64_t g_last_paint_time;

void paintLoadingOverlay(SDL_Renderer *renderer);

void resetLoadingOverlay();

#endif //AFLICKEROFSTILLNESS_LOADING_OVERLAY_PAINTER_H
