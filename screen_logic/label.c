#include "label.h"

#include <stdio.h>
#include <stdlib.h>
#include "stb_truetype.h"
#include "painter.h"

void initLabel(Label** label, const SDL_FRect rect, const SDL_Color fill_color, const LabelStyleData style_data,
        const LabelTextData text_data) {
    *label = malloc(sizeof(Label));
    if (*label == NULL) {
        printf("Error allocating label\n");
        return;
    }
    (*label)->rect = rect;
    (*label)->fill_color = fill_color;
    (*label)->style_data = style_data;
    (*label)->text_data = text_data;
}

void initDefaultLabelTextData(LabelTextData* text_data) {
    text_data->num_lines = 1;
    text_data->font = FONT_KLARISSA;
    text_data->font_size = 0;
    text_data->line_spacing = 0;
    text_data->vertical_text_margin = 0;
    text_data->horizontal_text_margin = 0;
    text_data->vertical_text_alignment = ALIGN_CENTER;
    text_data->horizontal_text_alignment = ALIGN_CENTER;
    text_data->text_color = (SDL_Color) {.r = 0, .g = 0, .b = 0, .a = 0};
    setLabelText(text_data->text, "", text_data->num_lines);
    setLineWidths(text_data->line_widths, text_data->text, FONT_KLARISSA, text_data->num_lines, 0);
    setLineHeight(&text_data->line_height, text_data->font_size, text_data->line_spacing, FONT_KLARISSA);
}

void initLabelTextData(LabelTextData* text_data, const char* text, const int font, const float font_size,
        const float line_spacing, const float vertical_text_margin, const float horizontal_text_margin,
        const int vertical_text_alignment, const int horizontal_text_alignment, const SDL_Color text_color) {
    text_data->num_lines = getNumLines(text);
    text_data->font = font;
    text_data->font_size = font_size;
    text_data->line_spacing = line_spacing;
    text_data->vertical_text_margin = vertical_text_margin;
    text_data->horizontal_text_margin = horizontal_text_margin;
    text_data->vertical_text_alignment = vertical_text_alignment;
    text_data->horizontal_text_alignment = horizontal_text_alignment;
    text_data->text_color = text_color;
    setLabelText(text_data->text, text, text_data->num_lines);
    setLineWidths(text_data->line_widths, text_data->text, font, text_data->num_lines, font_size);
    setLineHeight(&text_data->line_height, text_data->font_size, text_data->line_spacing, font);
}

void destroyLabel(Label** label) {
    free(*label);
    *label = NULL;
}

static void setLabelText(char label_text[50][100], const char* original_text, const int num_lines) {
    for (int i = 0; i < 50; i++) {
        label_text[i][0] = '\0';
    }
    if (num_lines == 0) {
        return;
    }
    int line = 0;
    int character = 0;
    for (int i = 0; original_text[i] != '\0' && line < num_lines; i++) {
        if (original_text[i] == '\n') {
            label_text[line][character] = '\0';
            line++;
            character = 0;
        } else if (original_text[i] == '\\' && original_text[i + 1] == 'n') {
            label_text[line][character] = '\0';
            line++;
            character = 0;
            i++;
        } else {
            label_text[line][character] = original_text[i];
            character++;
        }
    }
    if (line < num_lines) {
        label_text[line][character] = '\0';
    }
}

/*
 *----------------------------------------------------------------------------------------------------------------------
 * Painting
 *----------------------------------------------------------------------------------------------------------------------
 */

void paintLabel(SDL_Renderer* renderer, const Label *label) {
    if (label->style_data.bevel_radius == 0) {
        paintLabelRect(renderer, label);
        if (label->style_data.border_thickness > 0) {
            paintLabelBorder(renderer, label);
        }
    } else {
        paintBeveledLabelRect(renderer, label);
        if (label->style_data.border_thickness > 0) {
            paintBeveledLabelBorder(renderer, label);
        }
    }
    if (label->text_data.num_lines > 0) {
        paintLabelText(renderer, label);
    }
}

static void paintLabelText(SDL_Renderer* renderer, const Label* label) {
    if (label->text_data.text_color.a == 0) {
        return;
    }
    for (int i = 0; i < label->text_data.num_lines; i++) {
        SDL_FPoint p;
        if (label->text_data.horizontal_text_alignment == ALIGN_LEFT) {
            p.x = label->rect.x + label->text_data.horizontal_text_margin;
        } else if (label->text_data.horizontal_text_alignment == ALIGN_CENTER) {
            p.x = label->rect.x + label->text_data.horizontal_text_margin +
                (label->rect.w - label->text_data.line_widths[i]) / 2.0f;
        } else {
            p.x = label->rect.x + label->rect.w - label->text_data.line_widths[i] - label->text_data.horizontal_text_margin;
        }

        if (label->text_data.vertical_text_alignment == ALIGN_LEFT) {
            p.y = label->rect.y + label->text_data.vertical_text_margin + (float) i * label->text_data.line_height;
        } else if (label->text_data.vertical_text_alignment == ALIGN_CENTER) {
            p.y = label->rect.y + label->text_data.vertical_text_margin +
                (label->rect.h - label->text_data.line_height * (float) label->text_data.num_lines) / 2.0f +
                (float) i * label->text_data.line_height;
        } else {
            p.y = label->rect.y + label->rect.h - label->text_data.vertical_text_margin -
                label->text_data.line_height * (float) (label->text_data.num_lines - i);
        }
        paintText(renderer, label->text_data.text[i], p, label->text_data.font_size, label->text_data.text_color,
            label->text_data.font);
    }
}

static void paintLabelRect(SDL_Renderer* renderer, const Label* label) {
    if (label->fill_color.a == 0) {
        return;
    }
    SDL_SetRenderDrawColor(renderer, label->fill_color.r, label->fill_color.g, label->fill_color.b,
        label->fill_color.a);
    const SDL_FRect rect = {
        .x = label->rect.x + label->style_data.border_thickness,
        .y = label->rect.y + label->style_data.border_thickness,
        .w = label->rect.w - label->style_data.border_thickness * 2,
        .h = label->rect.h - label->style_data.border_thickness * 2
    };
    SDL_RenderFillRect(renderer, &rect);
}

static void paintBeveledLabelRect(SDL_Renderer* renderer, const Label* label) {
    if (label->fill_color.a == 0) {
        return;
    }
    SDL_SetRenderDrawColor(renderer, label->fill_color.r, label->fill_color.g, label->fill_color.b,
        label->fill_color.a);
    //top rect
    const SDL_FRect rect1 = {
        .x = label->rect.x + label->style_data.bevel_radius,
        .y = label->rect.y + label->style_data.border_thickness,
        .w = label->rect.w - label->style_data.bevel_radius * 2,
        .h = label->style_data.bevel_radius - label->style_data.border_thickness
    };
    SDL_RenderFillRect(renderer, &rect1);
    //middle rect
    const SDL_FRect rect2 = {
        .x = label->rect.x + label->style_data.border_thickness,
        .y = label->rect.y + label->style_data.bevel_radius,
        .w = label->rect.w - label->style_data.border_thickness * 2,
        .h = label->rect.h - label->style_data.bevel_radius * 2
    };
    SDL_RenderFillRect(renderer, &rect2);
    //bottom rect
    const SDL_FRect rect3 = {
        .x = label->rect.x + label->style_data.bevel_radius,
        .y = label->rect.y + label->rect.h - label->style_data.bevel_radius,
        .w = label->rect.w - label->style_data.bevel_radius * 2,
        .h = label->style_data.bevel_radius - label->style_data.border_thickness
    };
    SDL_RenderFillRect(renderer, &rect3);
    //upper left circle
    const Circle circle1 = {
        .x = label->rect.x + label->style_data.bevel_radius - 1.0f,
        .y = label->rect.y + label->style_data.bevel_radius - 1.0f,
        .inner_radius = 0,
        .outer_radius = label->style_data.bevel_radius - label->style_data.border_thickness
    };
    paintRightAngleSemiCircle(renderer, circle1, 2, label->fill_color);
    //upper right circle
    const Circle circle2 = {
        .x = label->rect.x + label->rect.w - label->style_data.bevel_radius,
        .y = label->rect.y + label->style_data.bevel_radius - 1.0f,
        .inner_radius = 0,
        .outer_radius = label->style_data.bevel_radius - label->style_data.border_thickness
    };
    paintRightAngleSemiCircle(renderer, circle2, 3, label->fill_color);
    //bottom right circle
    const Circle circle3 = {
        .x = label->rect.x + label->rect.w - label->style_data.bevel_radius,
        .y = label->rect.y + label->rect.h - label->style_data.bevel_radius,
        .inner_radius = 0,
        .outer_radius = label->style_data.bevel_radius - label->style_data.border_thickness
    };
    paintRightAngleSemiCircle(renderer, circle3, 0, label->fill_color);
    //bottom left circle
    const Circle circle4 = {
        .x = label->rect.x + label->style_data.bevel_radius - 1.0f,
        .y = label->rect.y + label->rect.h - label->style_data.bevel_radius,
        .inner_radius = 0,
        .outer_radius = label->style_data.bevel_radius - label->style_data.border_thickness
    };
    paintRightAngleSemiCircle(renderer, circle4, 1, label->fill_color);
}

static void paintLabelBorder(SDL_Renderer* renderer, const Label* label) {
    if (label->style_data.border_color.a == 0) {
        return;
    }
    SDL_SetRenderDrawColor(renderer, label->style_data.border_color.r, label->style_data.border_color.g,
        label->style_data.border_color.b, label->style_data.border_color.a);
    //top line
    const SDL_FRect rect1 = {
        .x = label->rect.x,
        .y = label->rect.y,
        .w = label->rect.w,
        .h = label->style_data.border_thickness
    };
    SDL_RenderFillRect(renderer, &rect1);
    //right line
    const SDL_FRect rect2 = {
        .x = label->rect.x + label->rect.w - label->style_data.border_thickness,
        .y = label->rect.y + label->style_data.border_thickness,
        .w = label->style_data.border_thickness,
        .h = label->rect.h - label->style_data.border_thickness * 2
    };
    SDL_RenderFillRect(renderer, &rect2);
    //bottom line
    const SDL_FRect rect3 = {
        .x = label->rect.x,
        .y = label->rect.y + label->rect.h - label->style_data.border_thickness,
        .w = label->rect.w,
        .h = label->style_data.border_thickness
    };
    SDL_RenderFillRect(renderer, &rect3);
    //left line
    const SDL_FRect rect4 = {
        .x = label->rect.x,
        .y = label->rect.y + label->style_data.border_thickness,
        .w = label->style_data.border_thickness,
        .h = label->rect.h - label->style_data.border_thickness * 2
    };
    SDL_RenderFillRect(renderer, &rect4);
}

static void paintBeveledLabelBorder(SDL_Renderer* renderer, const Label* label) {
    if (label->style_data.border_color.a == 0) {
        return;
    }
    SDL_SetRenderDrawColor(renderer, label->style_data.border_color.r, label->style_data.border_color.g,
        label->style_data.border_color.b, label->style_data.border_color.a);
    //top line
    const SDL_FRect rect1 = {
        .x = label->rect.x + label->style_data.bevel_radius,
        .y = label->rect.y,
        .w = label->rect.w - label->style_data.bevel_radius * 2,
        .h = label->style_data.border_thickness
    };
    SDL_RenderFillRect(renderer, &rect1);
    //right line
    const SDL_FRect rect2 = {
        .x = label->rect.x + label->rect.w - label->style_data.border_thickness,
        .y = label->rect.y + label->style_data.bevel_radius,
        .w = label->style_data.border_thickness,
        .h = label->rect.h - label->style_data.bevel_radius * 2
    };
    SDL_RenderFillRect(renderer, &rect2);
    //bottom line
    const SDL_FRect rect3 = {
        .x = label->rect.x + label->style_data.bevel_radius,
        .y = label->rect.y + label->rect.h - label->style_data.border_thickness,
        .w = label->rect.w - label->style_data.bevel_radius * 2,
        .h = label->style_data.border_thickness
    };
    SDL_RenderFillRect(renderer, &rect3);
    //left line
    const SDL_FRect rect4 = {
        .x = label->rect.x,
        .y = label->rect.y + label->style_data.bevel_radius,
        .w = label->style_data.border_thickness,
        .h = label->rect.h - label->style_data.bevel_radius * 2
    };
    SDL_RenderFillRect(renderer, &rect4);
    //upper left circle
    const Circle circle1 = {
        .x = label->rect.x + label->style_data.bevel_radius - 1.0f,
        .y = label->rect.y + label->style_data.bevel_radius - 1.0f,
        .inner_radius = label->style_data.bevel_radius - label->style_data.border_thickness,
        .outer_radius = label->style_data.bevel_radius
    };
    paintRightAngleSemiCircle(renderer, circle1, 2, label->style_data.border_color);
    //upper right circle
    const Circle circle2 = {
        .x = label->rect.x + label->rect.w - label->style_data.bevel_radius,
        .y = label->rect.y + label->style_data.bevel_radius - 1.0f,
        .inner_radius = label->style_data.bevel_radius - label->style_data.border_thickness,
        .outer_radius = label->style_data.bevel_radius
    };
    paintRightAngleSemiCircle(renderer, circle2, 3, label->style_data.border_color);
    //bottom right circle
    const Circle circle3 = {
        .x = label->rect.x + label->rect.w - label->style_data.bevel_radius,
        .y = label->rect.y + label->rect.h - label->style_data.bevel_radius,
        .inner_radius = label->style_data.bevel_radius - label->style_data.border_thickness,
        .outer_radius = label->style_data.bevel_radius
    };
    paintRightAngleSemiCircle(renderer, circle3, 0, label->style_data.border_color);
    //bottom left circle
    const Circle circle4 = {
        .x = label->rect.x + label->style_data.bevel_radius - 1.0f,
        .y = label->rect.y + label->rect.h - label->style_data.bevel_radius,
        .inner_radius = label->style_data.bevel_radius - label->style_data.border_thickness,
        .outer_radius = label->style_data.bevel_radius
    };
    paintRightAngleSemiCircle(renderer, circle4, 1, label->style_data.border_color);
}

/*
 *----------------------------------------------------------------------------------------------------------------------
 * Text Metrics
 *----------------------------------------------------------------------------------------------------------------------
 */

int getNumLines(const char* text) {
    if (text == NULL) {
        return 0;
    }
    int num_lines = 1;
    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] == '\n') {
            num_lines++;
        } else if (text[i] == '\\' && text[i + 1] == 'n') {
            num_lines++;
            i++;
        }
    }
    return num_lines;
}

void setLineWidths(float line_widths[50], const char (*text)[100], const int font, const int num_lines,
        const float font_size) {
    float scale;
    if (font == FONT_KLARISSA) {
        scale = stbtt_ScaleForPixelHeight(&g_font_klarissa, font_size);
    } else {
        scale = stbtt_ScaleForPixelHeight(&g_font_pixeloid, font_size);
    }
    for (int i = 0; i < num_lines; i++) {
        line_widths[i] = getLineWidth(text[i], scale, font);
    }
}

static float getLineWidth(const char* line, const float scale, const int font) {
    if (line[0] == '\0' || (font == FONT_KLARISSA && !g_font_buffer_klarissa) ||
            (font == FONT_PIXELOID && !g_font_buffer_pixeloid)) {
        return 0.0f;
    }
    float current_x = 0;
    int last_advance = 0;
    int last_c_x2 = 0;
    for (int i = 0; line[i] != '\0'; ++i) {
        // get horizontal g_font metrics
        int advance, lsb;
        int c_x1, c_y1, c_x2, c_y2;
        if (font == FONT_KLARISSA) {
            stbtt_GetCodepointHMetrics(&g_font_klarissa, line[i], &advance, &lsb);
            stbtt_GetCodepointBitmapBox(&g_font_klarissa, line[i], scale, scale, &c_x1, &c_y1,
                &c_x2, &c_y2);
        } else {
            stbtt_GetCodepointHMetrics(&g_font_pixeloid, line[i], &advance, &lsb);
            stbtt_GetCodepointBitmapBox(&g_font_pixeloid, line[i], scale, scale, &c_x1, &c_y1,
                &c_x2, &c_y2);
        }
        current_x += (float) advance * scale;
        if (line[i + 1] != '\0') {
            if (font == FONT_KLARISSA) {
                current_x += (float) stbtt_GetCodepointKernAdvance(&g_font_klarissa, line[i], line[i + 1]) *
                    scale;
            } else {
                current_x += (float) stbtt_GetCodepointKernAdvance(&g_font_pixeloid, line[i], line[i + 1]) *
                    scale;
            }
        }
        last_advance = advance;
        last_c_x2 = c_x2;
    }
    current_x -= (float) last_advance * scale;
    current_x += (float) last_c_x2;
    return current_x;
}

void setLineHeight(float* line_height, const float font_size, const float line_spacing, const int font) {
    if (font == FONT_KLARISSA && !g_font_buffer_klarissa || font == FONT_PIXELOID && !g_font_buffer_pixeloid) {
        return;
    }
    float scale;
    int ascent, descent, line_gap;
    if (font == FONT_KLARISSA) {
        scale = stbtt_ScaleForPixelHeight(&g_font_klarissa, font_size);
        stbtt_GetFontVMetrics(&g_font_klarissa, &ascent, &descent, &line_gap);
    } else {
        scale = stbtt_ScaleForPixelHeight(&g_font_pixeloid, font_size);
        stbtt_GetFontVMetrics(&g_font_pixeloid, &ascent, &descent, &line_gap);
    }
    *line_height = (float) (ascent - descent + line_gap) * scale + line_spacing;
}
