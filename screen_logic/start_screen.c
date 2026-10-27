#include "start_screen.h"

#include <stdlib.h>

#include "SDL3/SDL.h"
#include "SDL3_image/SDL_image.h"
#include "../constants.h"

void initStartScreen(StartScreen** start_screen) {
    *start_screen = malloc(sizeof(StartScreen));
    (*start_screen)->textures = malloc(sizeof(StartScreenTextures));
    (*start_screen)->surfaces = malloc(sizeof(StartScreenSurfaces));
    setStartScreenSurfaces(*start_screen);
    setStartScreenRects(*start_screen);
    setStartScreenLabels(*start_screen);
    setStartScreenButtons(*start_screen);
}

void setStartScreenTextures(const StartScreen* start_screen, SDL_Renderer* renderer) {
    start_screen->textures->background_texture =
        SDL_CreateTextureFromSurface(renderer, start_screen->surfaces->background_surface);
    SDL_DestroySurface(start_screen->surfaces->background_surface);
    start_screen->surfaces->background_surface = NULL;

    start_screen->textures->left_panel_texture =
        SDL_CreateTextureFromSurface(renderer, start_screen->surfaces->left_panel_surface);
    SDL_DestroySurface(start_screen->surfaces->left_panel_surface);
    start_screen->surfaces->left_panel_surface = NULL;

    start_screen->textures->right_panel_texture =
        SDL_CreateTextureFromSurface(renderer, start_screen->surfaces->right_panel_surface);
    SDL_DestroySurface(start_screen->surfaces->right_panel_surface);
    start_screen->surfaces->right_panel_surface = NULL;
}

void destroyStartScreen(StartScreen** start_screen) {
    if (*start_screen == NULL) return;
    destroyLabel(&(*start_screen)->game_label);
    destroyButton((*start_screen)->play_button);
    destroyButton((*start_screen)->settings_button);
    destroyButton((*start_screen)->exit_button);
    SDL_DestroyTexture((*start_screen)->textures->background_texture);
    SDL_DestroyTexture((*start_screen)->textures->left_panel_texture);
    SDL_DestroyTexture((*start_screen)->textures->right_panel_texture);
    free((*start_screen)->surfaces);
    (*start_screen)->surfaces = NULL;
    free((*start_screen)->textures);
    (*start_screen)->textures = NULL;
    free(*start_screen);
    *start_screen = NULL;
}

int paintStartScreen(const StartScreen* start_screen, SDL_Renderer* renderer) {
    SDL_RenderTexture(renderer, start_screen->textures->left_panel_texture, NULL,
        &start_screen->left_panel_rect);
    SDL_RenderTexture(renderer, start_screen->textures->right_panel_texture, NULL,
        &start_screen->right_panel_rect);
    paintLabel(renderer, start_screen->left_panel_label);
    paintLabel(renderer, start_screen->right_panel_label);
    const SDL_FRect background_rect = {
        .x = (float) (WINDOW_WIDTH - WINDOW_HEIGHT) / 2.0f,
        .y = 0.0f,
        .w = WINDOW_HEIGHT,
        .h = WINDOW_HEIGHT
    };
    SDL_RenderTexture(renderer, start_screen->textures->background_texture, NULL, &background_rect);
    paintLabel(renderer, start_screen->game_label);
    paintLabel(renderer, start_screen->play_button->label);
    paintLabel(renderer, start_screen->settings_button->label);
    paintLabel(renderer, start_screen->exit_button->label);
    return -1;
}

/*
 *----------------------------------------------------------------------------------------------------------------------
 * Event handling
 *----------------------------------------------------------------------------------------------------------------------
 */

void startScreenMousePressed(const StartScreen* start_screen, const SDL_FPoint mouse_pos) {
    if (buttonPressCheck(start_screen->play_button, mouse_pos)) return;
    if (buttonPressCheck(start_screen->settings_button, mouse_pos)) return;
    buttonPressCheck(start_screen->exit_button, mouse_pos);
}

int startScreenMouseReleased(const StartScreen* start_screen, const SDL_FPoint mouse_pos, bool* program_running) {
    if (buttonReleaseCheck(start_screen->play_button, mouse_pos)) {
        return SAVES_SCREEN;
    }
    if (buttonReleaseCheck(start_screen->settings_button, mouse_pos)) {
        return SETTINGS_SCREEN;
    }
    if (buttonReleaseCheck(start_screen->exit_button, mouse_pos)) {
        *program_running = false;
    }
    return -1;
}

void startScreenMouseDragged(const StartScreen* start_screen, const SDL_FPoint mouse_pos) {
    buttonMouseDragCheck(start_screen->play_button, mouse_pos);
    buttonMouseDragCheck(start_screen->settings_button, mouse_pos);
    buttonMouseDragCheck(start_screen->exit_button, mouse_pos);
}

void startScreenMouseMoved(const StartScreen* start_screen, SDL_FPoint mouse_pos) {

}

/*
 *----------------------------------------------------------------------------------------------------------------------
 * Helper functions
 *----------------------------------------------------------------------------------------------------------------------
 */

static void setStartScreenSurfaces(const StartScreen* start_screen) {
    start_screen->surfaces->background_surface = IMG_Load("game_data/textures/start_screen/background.png");
    if (!start_screen->surfaces->background_surface) {
        SDL_Log("Failed to load image: %s", SDL_GetError());
    }
    start_screen->surfaces->left_panel_surface = IMG_Load("game_data/textures/start_screen/left_panel.png");
    if (!start_screen->surfaces->left_panel_surface) {
        SDL_Log("Failed to load image: %s", SDL_GetError());
    }
    start_screen->surfaces->right_panel_surface = IMG_Load("game_data/textures/start_screen/right_panel.png");
    if (!start_screen->surfaces->right_panel_surface) {
        SDL_Log("Failed to load image: %s", SDL_GetError());
    }
}

static void setStartScreenRects(StartScreen* start_screen) {
    start_screen->left_panel_rect = (SDL_FRect) {
        .x = ((float) (WINDOW_WIDTH - WINDOW_HEIGHT) / 2.0f - (float) WINDOW_HEIGHT) / 2.0f,
        .y = 0.0f,
        .w = (float) WINDOW_HEIGHT,
        .h = (float) WINDOW_HEIGHT
    };
    start_screen->right_panel_rect = (SDL_FRect) {
        .x = (3.0f * (float) WINDOW_WIDTH - (float) WINDOW_HEIGHT) / 4.0f,
        .y = 0.0f,
        .w = (float) WINDOW_HEIGHT,
        .h = (float) WINDOW_HEIGHT
    };
}

static void setStartScreenLabels(StartScreen* start_screen) {
    /*
     * background labels
     */

    const SDL_FRect left_label_rect = {
        .x = 0.0f,
        .y = 0.0f,
        .w = (float) (WINDOW_WIDTH - WINDOW_HEIGHT) / 2.0f,
        .h = (float) WINDOW_HEIGHT
    };
    const SDL_FRect right_label_rect = {
        .x = (float) (WINDOW_WIDTH - WINDOW_HEIGHT) / 2.0f + (float) WINDOW_HEIGHT,
        .y = 0.0f,
        .w = (float) (WINDOW_WIDTH - WINDOW_HEIGHT) / 2.0f,
        .h = (float) WINDOW_HEIGHT
    };
    const LabelStyleData style_data = {
        .border_thickness = 10.0f,
        .bevel_radius = 0.0f,
        .border_color = {.r = 0, .g = 0, .b = 0, .a = 100}
    };
    LabelTextData text_data;
    initDefaultLabelTextData(&text_data);
    initLabel(&start_screen->left_panel_label, left_label_rect, (SDL_Color) {.r = 255, .g = 255, .b = 255, .a = 0},
        style_data, text_data);
    initLabel(&start_screen->right_panel_label, right_label_rect, (SDL_Color) {.r = 255, .g = 255, .b = 255, .a = 0},
        style_data, text_data);

    /*
     * game label
     */

    const LabelStyleData game_label_style_data = {
        .border_thickness = 10,
        .bevel_radius = 30,
        .border_color = {.r = 255, .g = 255, .b = 255, .a = 100}
    };
    LabelTextData game_label_text_data;
    const char* text = "A Flicker of Stillness:\nThe Video Game";
    const float font_size = 75.0f;
    const float line_spacing = 0.0f;
    const float vertical_text_margin = 0.0f;
    const float horizontal_text_margin = 0.0f;
    const int vertical_text_alignment = ALIGN_CENTER;
    const int horizontal_text_alignment = ALIGN_CENTER;
    const SDL_Color text_color = {.r = 255, .g = 255, .b = 255, .a = 100};
    initLabelTextData(&game_label_text_data, text, FONT_KLARISSA, font_size, line_spacing, vertical_text_margin,
        horizontal_text_margin, vertical_text_alignment, horizontal_text_alignment, text_color);
    const SDL_FRect rect = (SDL_FRect) {
        .x = (WINDOW_WIDTH - 800.0f) / 2.0f,
        .y = 115.0f,
        .w = 800.0f,
        .h = 220.0f
    };
    initLabel(&start_screen->game_label, rect, (SDL_Color) {.r = 75, .g = 50, .b = 50, .a = 100}, game_label_style_data,
        game_label_text_data);
}

static void setStartScreenButtons(StartScreen* start_screen) {
    //consistent data
    const float x = (WINDOW_WIDTH - 800.0f) / 2.0f;
    const float y = 365.0f;
    const float width = 800.0f;
    const float height = 120.0f;
    const float spacing = 30.0f;
    const SDL_Color fill_color = {.r = 75, .g = 50, .b = 50, .a = 100};
    const LabelStyleData style_data = {
        .border_thickness = 10,
        .bevel_radius = 30,
        .border_color = {.r = 255, .g = 255, .b = 255, .a = 100}
    };
    const float font_size = 70.0f;
    const float line_spacing = 0.0f;
    const float vertical_text_margin = 0.0f; const float horizontal_text_margin = 0.0f;
    const int vertical_text_alignment = ALIGN_CENTER; const int horizontal_text_alignment = ALIGN_CENTER;
    const SDL_Color text_color = {.r = 255, .g = 255, .b = 255, .a = 100};

    const SDL_Color click_color = {.r = 100, .g = 0, .b = 0, .a = 100};
    const ButtonColorData color_data = {
        .boarder_click_color = click_color,
        .boarder_idle_color = style_data.border_color,
        .text_click_color = click_color,
        .text_idle_color = text_color,
        .fill_click_color = fill_color,
        .fill_idle_color = fill_color,
    };
    //Play button
    LabelTextData play_label_text_data;
    const char* play_label_text = "Play";
    initLabelTextData(&play_label_text_data, play_label_text, FONT_KLARISSA, font_size, line_spacing,
        vertical_text_margin, horizontal_text_margin, vertical_text_alignment, horizontal_text_alignment, text_color);
    Label* play_label = NULL;
    initLabel(&play_label, (SDL_FRect) {.x = x, .y = y, .w = width, .h = height}, fill_color, style_data,
        play_label_text_data);
    initButton(&start_screen->play_button, play_label, color_data);
    //settings button
    LabelTextData settings_label_text_data;
    const char* settings_label_text = "Settings";
    initLabelTextData(&settings_label_text_data, settings_label_text, FONT_KLARISSA, font_size, line_spacing,
        vertical_text_margin, horizontal_text_margin, vertical_text_alignment, horizontal_text_alignment, text_color);
    Label* settings_label = NULL;
    initLabel(&settings_label, (SDL_FRect) {.x = x, .y = y + height + spacing, .w = width, .h = height}, fill_color,
        style_data, settings_label_text_data);
    initButton(&start_screen->settings_button, settings_label, color_data);
    //exit button
    LabelTextData exit_label_text_data;
    const char* exit_label_text = "Exit";
    initLabelTextData(&exit_label_text_data, exit_label_text, FONT_KLARISSA, font_size, line_spacing,
        vertical_text_margin, horizontal_text_margin, vertical_text_alignment, horizontal_text_alignment, text_color);
    Label* exit_label = NULL;
    initLabel(&exit_label, (SDL_FRect) {.x = x, .y = y + (height + spacing) * 2, .w = width, .h = height}, fill_color,
        style_data, exit_label_text_data);
    initButton(&start_screen->exit_button, exit_label, color_data);
}