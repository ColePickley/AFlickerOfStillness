#define STB_TRUETYPE_IMPLEMENTATION

#include "painter.h"
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t* g_font_buffer;
stbtt_fontinfo g_font;

bool initPainterFont(void) {
    const char* font_path = "fonts/TimesNewRoman.ttf";
    FILE* font_file = fopen(font_path, "rb");
    if (!font_file) {
        fprintf(stderr, "Failed to open g_font file: %s\n", font_path);
        return false;
    }

    fseek(font_file, 0, SEEK_END);
    long size = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);

    g_font_buffer = malloc(size);
    if (!g_font_buffer) {
        fclose(font_file);
        return false;
    }

    fread(g_font_buffer, 1, size, font_file);
    fclose(font_file);

    if (!stbtt_InitFont(&g_font, g_font_buffer, stbtt_GetFontOffsetForIndex(g_font_buffer, 0))) {
        fprintf(stderr, "Failed to initialize standard g_font structure.\n");
        free(g_font_buffer);
        g_font_buffer = NULL;
        return false;
    }
    return true;
}

void destroyPainterFont() {
    if (g_font_buffer) {
        free(g_font_buffer);
        g_font_buffer = NULL;
    }
}

void paintEllipse(SDL_Renderer* renderer, const Ellipse e, const SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int x = (int) (e.x - e.rx); x <= (int) (e.x + e.rx); x++) {
        for (int y = (int) (e.y - e.ry); y <= (int) (e.y + e.ry); y++) {
            if (pow((float) y - e.y, 2) / pow(e.ry, 2) +
                    pow((float) x - e.x, 2) / pow(e.rx, 2) <= 1) {
                SDL_RenderPoint(renderer, (float) x, (float) y);
            }
        }
    }
}

void paintRightAngleSemiCircle(SDL_Renderer* renderer, const Circle circle, const int quadrant,
        const SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    float start_x, end_x, start_y, end_y;
    switch (quadrant) {
        case 0: {
            start_x = circle.x;
            end_x = circle.x + circle.outer_radius;
            start_y = circle.y;
            end_y = circle.y + circle.outer_radius;
            break;
        }
        case 1: {
            start_x = circle.x - circle.outer_radius;
            end_x = circle.x;
            start_y = circle.y;
            end_y = circle.y + circle.outer_radius;
            break;
        }
        case 2: {
            start_x = circle.x - circle.outer_radius;
            end_x = circle.x;
            start_y = circle.y - circle.outer_radius;
            end_y = circle.y;
            break;
        }
        case 3: {
            start_x = circle.x;
            end_x = circle.x + circle.outer_radius;
            start_y = circle.y - circle.outer_radius;
            end_y = circle.y;
            break;
        }
        default: {
            return;
        }
    }
    const float outer_radius_squared = circle.outer_radius * circle.outer_radius;
    const float inner_radius_squared = circle.inner_radius * circle.inner_radius;
    for (int x = (int) start_x; x <= (int) end_x; x++) {
        for (int y = (int) start_y; y <= (int) end_y; y++) {
            const double distance_squared = pow((float) x - circle.x, 2) + pow((float) y - circle.y, 2);
            if (distance_squared < outer_radius_squared && distance_squared >= inner_radius_squared) {
                SDL_RenderPoint(renderer, (float) x, (float) y);
            }
        }
    }
}

void paintPolygon(SDL_Renderer* renderer, const SDL_FPoint pts[], const int num_pts, const SDL_Color color) {

}

void paintText(SDL_Renderer* renderer, const char* text, const SDL_FPoint p, const float font_size,
        const SDL_Color color) {
    if (!g_font_buffer) {
        return;
    }
    // get vertical g_font metrics
    const float scale = stbtt_ScaleForPixelHeight(&g_font, font_size);
    int ascent, descent, line_gap;
    stbtt_GetFontVMetrics(&g_font, &ascent, &descent, &line_gap);
    const int base_line = (int) p.y + (int) ((float) ascent * scale);
    // paint individual characters
    float current_x = p.x;
    for (int i = 0; text[i] != '\0'; ++i) {
        int advance, lsb;
        stbtt_GetCodepointHMetrics(&g_font, text[i], &advance, &lsb);
        int c_x1, c_y1, c_x2, c_y2;
        stbtt_GetCodepointBitmapBox(&g_font, text[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);
        const int glyph_w = c_x2 - c_x1;
        const int glyph_h = c_y2 - c_y1;
        if (glyph_w > 0 && glyph_h > 0) {
            uint8_t* bitmap = malloc(glyph_w * glyph_h);
            stbtt_MakeCodepointBitmap(&g_font, bitmap, glyph_w, glyph_h, glyph_w, scale, scale, text[i]);
            for (int gy = 0; gy < glyph_h; ++gy) {
                for (int gx = 0; gx < glyph_w; ++gx) {
                    const uint8_t alpha = (int) ((float) bitmap[gy * glyph_w + gx] * (float) color.a / 255.0f);
                    const int screen_x = (int)current_x + c_x1 + gx;
                    const int screen_y = base_line + c_y1 + gy;
                    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha);
                    SDL_RenderPoint(renderer, (float) screen_x, (float) screen_y);
                }
            }
            free(bitmap);
        }
        current_x += (float) advance * scale;
        if (text[i + 1] != '\0') {
            current_x += (float) stbtt_GetCodepointKernAdvance(&g_font, text[i], text[i + 1]) * scale;
        }
    }
}
