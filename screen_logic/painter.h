#ifndef AFLICKEROFSTILLNESS_PAINTER_H
#define AFLICKEROFSTILLNESS_PAINTER_H
#include <SDL3/SDL.h>

#include "stb_truetype.h"

enum {
    FONT_KLARISSA, FONT_PIXELOID
};

typedef struct {
    float x, y, rx, ry;
}Ellipse;

typedef struct {
    float x, y, inner_radius, outer_radius;
}Circle;

extern uint8_t* g_font_buffer_klarissa;
extern stbtt_fontinfo g_font_klarissa;
extern uint8_t* g_font_buffer_pixeloid;
extern stbtt_fontinfo g_font_pixeloid;

bool initPainterFonts();

void destroyPainterFonts();

void paintEllipse(SDL_Renderer* renderer, Ellipse e, SDL_Color color);

void paintRightAngleSemiCircle(SDL_Renderer* renderer, Circle circle, int quadrant, SDL_Color color);

void paintText(SDL_Renderer* renderer, const char* text, SDL_FPoint p, float font_size, SDL_Color color, int font);

#endif //AFLICKEROFSTILLNESS_PAINTER_H
