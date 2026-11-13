#define STB_TRUETYPE_IMPLEMENTATION

#include "painter.h"
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t* g_font_buffer_klarissa;
stbtt_fontinfo g_font_klarissa;
uint8_t* g_font_buffer_pixeloid;
stbtt_fontinfo g_font_pixeloid;

bool initPainterFonts() {

    /*
     * Klarissa
     */

    const char* font_path_1 = "fonts/klarissa.regular.ttf";
    FILE* font_file_1 = fopen(font_path_1, "rb");
    if (!font_file_1) {
        fprintf(stderr, "Failed to open g_font file: %s\n", font_path_1);
        return false;
    }

    fseek(font_file_1, 0, SEEK_END);
    const long size_1 = ftell(font_file_1);
    fseek(font_file_1, 0, SEEK_SET);

    g_font_buffer_klarissa = malloc(size_1);
    if (!g_font_buffer_klarissa) {
        fclose(font_file_1);
        return false;
    }

    fread(g_font_buffer_klarissa, 1, size_1, font_file_1);
    fclose(font_file_1);

    if (!stbtt_InitFont(&g_font_klarissa, g_font_buffer_klarissa,
            stbtt_GetFontOffsetForIndex(g_font_buffer_klarissa, 0))) {
        fprintf(stderr, "Failed to initialize standard g_font structure.\n");
        free(g_font_buffer_klarissa);
        g_font_buffer_klarissa = NULL;
        return false;
    }

    /*
     * Cariopixel
     */

    const char* font_path_2 = "fonts/pixeloid.sans-bold.ttf";
    FILE* font_file_2 = fopen(font_path_2, "rb");
    if (!font_file_2) {
        fprintf(stderr, "Failed to open g_font file: %s\n", font_path_2);
        return false;
    }

    fseek(font_file_2, 0, SEEK_END);
    const long size_2 = ftell(font_file_2);
    fseek(font_file_2, 0, SEEK_SET);

    g_font_buffer_pixeloid = malloc(size_2);
    if (!g_font_buffer_pixeloid) {
        fclose(font_file_2);
        return false;
    }

    fread(g_font_buffer_pixeloid, 1, size_2, font_file_2);
    fclose(font_file_2);

    if (!stbtt_InitFont(&g_font_pixeloid, g_font_buffer_pixeloid,
            stbtt_GetFontOffsetForIndex(g_font_buffer_pixeloid, 0))) {
        fprintf(stderr, "Failed to initialize standard g_font structure.\n");
        free(g_font_buffer_pixeloid);
        g_font_buffer_pixeloid = NULL;
        return false;
    }

    return true;
}

void destroyPainterFonts() {
    if (g_font_buffer_klarissa) {
        free(g_font_buffer_klarissa);
        g_font_buffer_klarissa = NULL;
    }
    if (g_font_buffer_pixeloid) {
        free(g_font_buffer_pixeloid);
        g_font_buffer_pixeloid = NULL;
    }
}

void paintPolygon(SDL_Renderer* renderer, const int num_pts, const SDL_FPoint points[num_pts], const SDL_Color color) {
    SDL_Vertex vertices[num_pts];
    for (int i = 0; i < num_pts; i++) {
        vertices[i].position = points[i];
        vertices[i].color = (SDL_FColor) {
            .r = (float) color.r / 255.0f,
            .g = (float) color.g / 255.0f,
            .b = (float) color.b / 255.0f,
            .a = (float) color.a / 255.0f
        };
        vertices[i].tex_coord = (SDL_FPoint) {.x = 0.0f, .y = 0.0f};
    }
    SDL_RenderGeometry(renderer, NULL, vertices, num_pts, NULL, 0);
}

void paintFilledEllipse(SDL_Renderer* renderer, const Ellipse e, const SDL_Color color) {
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

void paintFilledCircle(SDL_Renderer* renderer, const Circle c, const SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    const double outer_radius_squared = pow(c.outer_radius, 2);
    const double inner_radius_squared = pow(c.inner_radius, 2);
    for (int x = (int) (c.x - c.outer_radius); x <= (int) (c.x + c.outer_radius); x++) {
        for (int y = (int) (c.y - c.outer_radius); y <= (int) (c.y + c.outer_radius); y++) {
            const double check = pow((float) y - c.y, 2) + pow((float) x - c.x, 2);
            if (check <= outer_radius_squared && check >= inner_radius_squared) {
                SDL_RenderPoint(renderer, (float) x, (float) y);
            }
        }
    }
}

void paintSemiCircle(SDL_Renderer* renderer, const SemiCircle sc, const SDL_Color color) {
    if (!renderer || sc.num_segments < 1 || sc.c.outer_radius <= 0.0f || sc.c.inner_radius < 0.0f ||
            sc.c.inner_radius >= sc.c.outer_radius) {
        return;
    }
    const float start_angle = (float) toRadians(sc.start_angle);
    const float end_angle = (float) toRadians(sc.end_angle);

    const int num_vertices = (sc.num_segments + 1) * 2;
    const int num_indices = sc.num_segments * 6;

    SDL_Vertex *vertices = SDL_malloc(sizeof(SDL_Vertex) * num_vertices);
    int *indices = SDL_malloc(sizeof(int) * num_indices);
    if (!vertices || !indices) {
        SDL_free(vertices);
        SDL_free(indices);
        return;
    }
    const float angle_step = (end_angle - start_angle) / (float) sc.num_segments;
    for (int i = 0; i <= sc.num_segments; ++i) {
        const float angle = start_angle + (float) i * angle_step;
        const float cos_a = cosf(angle);
        const float sin_a = sinf(angle);
        const int v_idx = i * 2;

        vertices[v_idx].position.x = sc.c.x + sc.c.inner_radius * cos_a;
        vertices[v_idx].position.y = sc.c.y + sc.c.inner_radius * sin_a;
        vertices[v_idx].color = (SDL_FColor) {
            .r = (float) color.r / 255.0f,
            .g = (float) color.g / 255.0f,
            .b = (float) color.b / 255.0f,
            .a = (float) color.a / 255.0f
        };
        vertices[v_idx].tex_coord.x = 0.0f;
        vertices[v_idx].tex_coord.y = 0.0f;

        vertices[v_idx + 1].position.x = sc.c.x + sc.c.outer_radius * cos_a;
        vertices[v_idx + 1].position.y = sc.c.y + sc.c.outer_radius * sin_a;
        vertices[v_idx + 1].color = (SDL_FColor) {
            .r = (float) color.r / 255.0f,
            .g = (float) color.g / 255.0f,
            .b = (float) color.b / 255.0f,
            .a = (float) color.a / 255.0f
        };
        vertices[v_idx + 1].tex_coord.x = 0.0f;
        vertices[v_idx + 1].tex_coord.y = 0.0f;
    }
    for (int i = 0; i < sc.num_segments; ++i) {
        const int i_idx = i * 6;
        const int v0 = i * 2;
        const int v1 = v0 + 1;
        const int v2 = v0 + 2;
        const int v3 = v0 + 3;
        indices[i_idx + 0] = v0;
        indices[i_idx + 1] = v1;
        indices[i_idx + 2] = v2;
        indices[i_idx + 3] = v1;
        indices[i_idx + 4] = v3;
        indices[i_idx + 5] = v2;
    }
    SDL_RenderGeometry(renderer, NULL, vertices, num_vertices, indices, num_indices);
    SDL_free(vertices);
    SDL_free(indices);
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

void paintText(SDL_Renderer* renderer, const char* text, const SDL_FPoint p, const float font_size,
        const SDL_Color color, const int font) {
    if (font == FONT_KLARISSA && !g_font_buffer_klarissa || font == FONT_PIXELOID && !g_font_buffer_pixeloid) {
        return;
    }
    // get vertical g_font metrics
    float scale;
    if (font == FONT_KLARISSA) {
        scale = stbtt_ScaleForPixelHeight(&g_font_klarissa, font_size);
    } else {
        scale = stbtt_ScaleForPixelHeight(&g_font_pixeloid, font_size);
    }
    int ascent, descent, line_gap;
    if (font == FONT_KLARISSA) {
        stbtt_GetFontVMetrics(&g_font_klarissa, &ascent, &descent, &line_gap);
    } else {
        stbtt_GetFontVMetrics(&g_font_pixeloid, &ascent, &descent, &line_gap);
    }
    const int base_line = (int) p.y + (int) ((float) ascent * scale);
    // paint individual characters
    float current_x = p.x;
    for (int i = 0; text[i] != '\0'; ++i) {
        int advance, lsb;
        int c_x1, c_y1, c_x2, c_y2;
        if (font == FONT_KLARISSA) {
            stbtt_GetCodepointHMetrics(&g_font_klarissa, text[i], &advance, &lsb);
            stbtt_GetCodepointBitmapBox(&g_font_klarissa, text[i], scale, scale, &c_x1, &c_y1,
                &c_x2, &c_y2);
        } else {
            stbtt_GetCodepointHMetrics(&g_font_pixeloid, text[i], &advance, &lsb);
            stbtt_GetCodepointBitmapBox(&g_font_pixeloid, text[i], scale, scale, &c_x1, &c_y1,
                &c_x2, &c_y2);
        }
        const int glyph_w = c_x2 - c_x1;
        const int glyph_h = c_y2 - c_y1;
        if (glyph_w > 0 && glyph_h > 0) {
            uint8_t* bitmap = malloc(glyph_w * glyph_h);
            if (font == FONT_KLARISSA) {
                stbtt_MakeCodepointBitmap(&g_font_klarissa, bitmap, glyph_w, glyph_h, glyph_w, scale,
                    scale, text[i]);
            } else {
                stbtt_MakeCodepointBitmap(&g_font_pixeloid, bitmap, glyph_w, glyph_h, glyph_w, scale,
                    scale, text[i]);
            }
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
            if (font == FONT_KLARISSA) {
                current_x += (float) stbtt_GetCodepointKernAdvance(&g_font_klarissa, text[i], text[i + 1]) *
                    scale;
            } else {
                current_x += (float) stbtt_GetCodepointKernAdvance(&g_font_pixeloid, text[i], text[i + 1]) *
                    scale;
            }
        }
    }
}
