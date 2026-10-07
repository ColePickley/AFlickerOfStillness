#ifndef AFLICKEROFSTILLNESS_PAINTER_H
#define AFLICKEROFSTILLNESS_PAINTER_H
#include <SDL3/SDL.h>

#include "stb_truetype.h"

typedef struct {
    float x, y, rx, ry;
}Ellipse;

typedef struct {
    float x, y, inner_radius, outer_radius;
}Circle;

extern uint8_t* g_font_buffer;
extern stbtt_fontinfo g_font;

bool initPainterFont();

void destroyPainterFont();

void paintEllipse(SDL_Renderer* renderer, Ellipse e, SDL_Color color);

void paintPolygon(SDL_Renderer* renderer, const SDL_FPoint pts[], int num_pts, SDL_Color color);

void paintRightAngleSemiCircle(SDL_Renderer* renderer, Circle circle, int quadrant, SDL_Color color);

void paintText(SDL_Renderer* renderer, const char* text, SDL_FPoint p, float font_size, SDL_Color color);

#endif //AFLICKEROFSTILLNESS_PAINTER_H
