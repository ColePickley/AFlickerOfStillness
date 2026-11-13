#ifndef AFLICKEROFSTILLNESS_PAINTER_H
#define AFLICKEROFSTILLNESS_PAINTER_H
#include <SDL3/SDL.h>
#include "../functions.h"
#include "stb_truetype.h"

#define MAX_ARC_SEGMENTS 32

enum {
    FONT_KLARISSA, FONT_PIXELOID
};

typedef struct {
    float x, y, rx, ry;
}Ellipse;

typedef struct {
    float x, y, inner_radius, outer_radius;
}Circle;

typedef struct {
    Circle c;
    float start_angle, end_angle;
    int num_segments;
}SemiCircle;

extern uint8_t* g_font_buffer_klarissa;
extern stbtt_fontinfo g_font_klarissa;
extern uint8_t* g_font_buffer_pixeloid;
extern stbtt_fontinfo g_font_pixeloid;

bool initPainter();

void destroyPainter();

void paintPolygon(SDL_Renderer* renderer, int num_pts, const SDL_FPoint points[num_pts], SDL_Color color);

void paintFilledEllipse(SDL_Renderer* renderer, Ellipse e, SDL_Color color);

void paintFilledCircle(SDL_Renderer* renderer, Circle c, SDL_Color color);

void paintSemiCircle(SDL_Renderer* renderer, SemiCircle sc, SDL_Color color);

void paintRightAngleSemiCircle(SDL_Renderer* renderer, Circle circle, int quadrant, SDL_Color color);

void paintText(SDL_Renderer* renderer, const char* text, SDL_FPoint p, float font_size, SDL_Color color, int font);

#endif //AFLICKEROFSTILLNESS_PAINTER_H
