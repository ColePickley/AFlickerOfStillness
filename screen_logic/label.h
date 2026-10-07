#ifndef AFLICKEROFSTILLNESS_LABEL_H
#define AFLICKEROFSTILLNESS_LABEL_H
#include <SDL3/SDL.h>

enum {
    ALIGN_LEFT = 0, ALIGN_TOP = 0, ALIGN_CENTER = 1, ALIGN_RIGHT = 2, ALIGN_BOTTOM = 2
};

typedef struct {
    float boarder_thickness;
    float bevel_radius;
    SDL_Color boarder_color;
}LabelStyleData;

typedef struct {
    int num_lines;
    float font_size;
    float line_spacing;
    char text[50][100];
    float line_widths[50];
    float line_height;
    float vertical_text_margin, horizontal_text_margin;
    int vertical_text_alignment, horizontal_text_alignment;
    SDL_Color text_color;
}LabelTextData;

typedef struct {
    SDL_FRect rect;
    SDL_Color fill_color;
    LabelStyleData style_data;
    LabelTextData text_data;
}Label;

void paintLabel(SDL_Renderer* renderer, const Label* label);

void initLabel(Label** label, SDL_FRect rect, SDL_Color fill_color, LabelStyleData style_data, LabelTextData text_data);

void initLabelTextData(LabelTextData* text_data, const char* text, float font_size, float line_spacing,
    float vertical_text_margin, float horizontal_text_margin, int vertical_text_alignment, int horizontal_text_alignment,
    SDL_Color text_color);

void destroyLabel(Label* label);

int getNumLines(const char* text);

static void setLabelText(char label_text[50][100], const char* original_text, int num_lines);

static void paintLabelText(SDL_Renderer* renderer, const Label* label);

static void paintLabelRect(SDL_Renderer* renderer, const Label* label);

static void paintBeveledLabelRect(SDL_Renderer* renderer, const Label* label);

static void paintLabelBoarder(SDL_Renderer* renderer, const Label* label);

static void paintBeveledLabelBoarder(SDL_Renderer* renderer, const Label* label);

static void setLineWidths(float line_widths[50], const char (*text)[100], int num_lines, float font_size);

static float getLineWidth(const char line[100], float scale);

void setLineHeight(float* line_height, float font_size, float line_spacing);

#endif //AFLICKEROFSTILLNESS_LABEL_H
