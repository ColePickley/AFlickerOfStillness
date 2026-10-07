#include "settings_screen.h"

#include <math.h>
#include <stdio.h>
#include <SDL3_image/SDL_image.h>
#include "../constants.h"
#include "../functions.h"
#include "start_screen.h"

void initSettingsScreen(SettingsScreen** settings_screen, SDL_Renderer* renderer, const UserData* user_data) {
    *settings_screen = malloc(sizeof(SettingsScreen));
    setSettingsScreenBackground(*settings_screen, renderer);
    setSettingsScreenLabels(*settings_screen, user_data);
    setSettingsScreenButtons(*settings_screen, user_data);
    setSettingsScreenRects(*settings_screen);
    (*settings_screen)->last_paint_time = 0;
    (*settings_screen)->do_entrance_animation = false;
    (*settings_screen)->do_exit_animation = false;
    (*settings_screen)->left_background_rect = (SDL_FRect) {.x = -2.0f * WINDOW_WIDTH / 3, .y = 0,
        .w = WINDOW_WIDTH, .h = WINDOW_HEIGHT};
    (*settings_screen)->right_background_rect = (SDL_FRect) {.x = 2.0f * WINDOW_WIDTH / 3, .y = 0,
        .w = WINDOW_WIDTH, .h = WINDOW_HEIGHT};
}

void resetSettingsScreen(SettingsScreen* settings_screen, const UserData* user_data) {
    setSettingsScreenLabels(settings_screen, user_data);
    setSettingsScreenButtons(settings_screen, user_data);
    setSettingsScreenRects(settings_screen);
}

void destroySettingsScreen(SettingsScreen* settings_screen) {
    destroyLabel(settings_screen->settings_label);
    destroyLabel(settings_screen->fps_label);
    destroyLabel(settings_screen->graphics_quality_label);
    destroyLabel(settings_screen->sensitivity_label);
    destroyButton(settings_screen->fullscreen_button);
    destroyButton(settings_screen->fps_button);
    destroyButton(settings_screen->graphics_quality_button);
    destroyButton(settings_screen->sensitivity_button);
    destroyButton(settings_screen->back_button);
    destroyButton(settings_screen->reset_button);
    destroyButton(settings_screen->save_button);
    SDL_DestroyTexture(settings_screen->left_background_texture);
    SDL_DestroyTexture(settings_screen->right_background_texture);
    free(settings_screen);
    settings_screen = NULL;
}

void paintSettingsScreen(SettingsScreen* settings_screen, SDL_Renderer* renderer, bool* paint_settings_screen,
        bool* paint_prev_screen) {

    /*
     * checking for entrance animation
     */

    if (settings_screen->last_paint_time == 0) {
        settings_screen->last_paint_time = SDL_GetTicks();
        settings_screen->do_entrance_animation = true;
    }

    /*
     * entrance animation
     */

    if (settings_screen->left_background_rect.x < 0 && settings_screen->do_entrance_animation) {
        const uint64_t time_elapsed = SDL_GetTicks() - settings_screen->last_paint_time;
        settings_screen->left_background_rect.x += (float) time_elapsed * 1.7f;
        settings_screen->right_background_rect.x -= (float) time_elapsed * 1.7f;
        if (settings_screen->left_background_rect.x > 0) {
            settings_screen->left_background_rect.x = 0;
            settings_screen->right_background_rect.x = 0;
        }
        if (settings_screen->left_background_rect.x == 0) {
            settings_screen->do_entrance_animation = false;
        }
    }

    /*
     * exit animation
     */

    if (settings_screen->left_background_rect.x > -2.0f * WINDOW_WIDTH / 3 && settings_screen->do_exit_animation) {
        const uint64_t time_elapsed = SDL_GetTicks() - settings_screen->last_paint_time;
        settings_screen->left_background_rect.x -= (float) time_elapsed * 1.7f;
        settings_screen->right_background_rect.x += (float) time_elapsed * 1.7f;
        if (settings_screen->left_background_rect.x < -2.0f * WINDOW_WIDTH / 3) {
            settings_screen->left_background_rect.x = -2.0f * WINDOW_WIDTH / 3;
            settings_screen->right_background_rect.x = 2.0f * WINDOW_WIDTH / 3;
        }
        if (settings_screen->left_background_rect.x == -2.0f * WINDOW_WIDTH / 3) {
            settings_screen->do_exit_animation = false;
            settings_screen->last_paint_time = 0;
            *paint_settings_screen = false;
        }
    }

    /*
     * checking if an animation has ended
     */

    if (*paint_prev_screen && !settings_screen->do_entrance_animation && !settings_screen->do_exit_animation &&
            settings_screen->last_paint_time != 0) {
        *paint_prev_screen = false;
    } else if (!*paint_prev_screen && settings_screen->do_exit_animation) {
        *paint_prev_screen = true;
    }

    /*
     * drawing background
     */

    SDL_RenderTexture(renderer, settings_screen->left_background_texture, NULL,
        &settings_screen->left_background_rect);
    SDL_RenderTexture(renderer, settings_screen->right_background_texture, NULL,
        &settings_screen->right_background_rect);

    /*
     * drawing settings
     */

    if (!settings_screen->do_entrance_animation && !settings_screen->do_exit_animation &&
            settings_screen->last_paint_time != 0) {
        paintLabel(renderer, settings_screen->settings_label);
        paintLabel(renderer, settings_screen->fps_label);
        paintLabel(renderer, settings_screen->graphics_quality_label);
        paintLabel(renderer, settings_screen->sensitivity_label);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 150);
        for (int i = 0; i < 5; i++) {
            SDL_RenderFillRect(renderer, &settings_screen->fps_slider_rects[i]);
            SDL_RenderFillRect(renderer, &settings_screen->graphics_quality_slider_rects[i]);
        }
        SDL_RenderFillRect(renderer, &settings_screen->sensitivity_slider_rect);

        paintLabel(renderer, settings_screen->fps_button->label);
        paintLabel(renderer, settings_screen->graphics_quality_button->label);
        paintLabel(renderer, settings_screen->sensitivity_button->label);

        paintLabel(renderer, settings_screen->fullscreen_button->label);
        paintLabel(renderer, settings_screen->back_button->label);
        paintLabel(renderer, settings_screen->reset_button->label);
        paintLabel(renderer, settings_screen->save_button->label);
    }

    /*
     * recording the end time for the next animation cycle
     */

    if (settings_screen->last_paint_time != 0) {
        settings_screen->last_paint_time = SDL_GetTicks();
    }
}

void settingsScreenMousePressed(const SettingsScreen* settings_screen, const SDL_FPoint mouse_pos) {
    if (isPointWithinButton(settings_screen->fps_button, mouse_pos)) {
        turnOnButton(settings_screen->fps_button);
        settings_screen->fps_button->is_selected = true;
    } else if (isPointWithinButton(settings_screen->graphics_quality_button, mouse_pos)) {
        turnOnButton(settings_screen->graphics_quality_button);
        settings_screen->graphics_quality_button->is_selected = true;
    } else if (isPointWithinButton(settings_screen->sensitivity_button, mouse_pos)) {
        turnOnButton(settings_screen->sensitivity_button);
        settings_screen->sensitivity_button->is_selected = true;
    } else if (isPointWithinButton(settings_screen->fullscreen_button, mouse_pos)) {
        turnOnButton(settings_screen->fullscreen_button);
        settings_screen->fullscreen_button->is_selected = true;
    } else if (isPointWithinButton(settings_screen->back_button, mouse_pos)) {
        turnOnButton(settings_screen->back_button);
        settings_screen->back_button->is_selected = true;
    } else if (isPointWithinButton(settings_screen->reset_button, mouse_pos)) {
        turnOnButton(settings_screen->reset_button);
        settings_screen->reset_button->is_selected = true;
    } else if (isPointWithinButton(settings_screen->save_button, mouse_pos)) {
        turnOnButton(settings_screen->save_button);
        settings_screen->save_button->is_selected = true;
    }
}

void settingsScreenMouseReleased(SettingsScreen* settings_screen, const SDL_FPoint mouse_pos, UserData* user_data,
        SDL_Window* window) {
    turnOffButton(settings_screen->fps_button);
    turnOffButton(settings_screen->graphics_quality_button);
    turnOffButton(settings_screen->sensitivity_button);
    turnOffButton(settings_screen->fullscreen_button);
    turnOffButton(settings_screen->back_button);
    turnOffButton(settings_screen->reset_button);
    turnOffButton(settings_screen->save_button);

    if (settings_screen->fps_button->is_selected) {
        float adjusted_x = mouse_pos.x - ((WINDOW_WIDTH - 800.0f) / 2.0f + 30.0f);
        adjusted_x = max(0.0f, min(730.0f, adjusted_x));
        const int slider_state = (int) round((double) adjusted_x / 730.0 * 2.0);
        int frame_rate_cap = 15;
        if (slider_state == 1) {
            frame_rate_cap = 30;
        } else if (slider_state == 2) {
            frame_rate_cap = 60;
        }
        user_data->frame_rate_cap = frame_rate_cap;
        resetSettingsScreen(settings_screen, user_data);
    } else if (settings_screen->fps_button->is_selected) {
        float adjusted_x = mouse_pos.x - ((WINDOW_WIDTH - 800.0f) / 2.0f + 30.0f);
        adjusted_x = max(0.0f, min(730.0f, adjusted_x));
        user_data->graphics_quality = (int) round((double) adjusted_x / 730.0 * 2.0);
        resetSettingsScreen(settings_screen, user_data);
    } else if (settings_screen->sensitivity_button->is_selected) {
        float adjusted_x = mouse_pos.x - ((WINDOW_WIDTH - 800.0f) / 2.0f + 30.0f);
        adjusted_x = max(0.0f, min(730.0f, adjusted_x));
        user_data->sensitivity = (int) (adjusted_x / 730.0f * 100.0f);
        resetSettingsScreen(settings_screen, user_data);
    } else if (isPointWithinButton(settings_screen->fullscreen_button, mouse_pos) &&
            settings_screen->fullscreen_button->is_selected) {
        user_data->fullscreen = !user_data->fullscreen;
        SDL_SetWindowFullscreen(window, user_data->fullscreen);
        resetSettingsScreen(settings_screen, user_data);
    } else if (isPointWithinButton(settings_screen->back_button, mouse_pos) && settings_screen->back_button->is_selected) {
        settings_screen->do_exit_animation = true;
    } else if (isPointWithinButton(settings_screen->reset_button, mouse_pos) && settings_screen->reset_button->is_selected) {
        resetUserData(user_data);
        SDL_SetWindowFullscreen(window, user_data->fullscreen);
        resetSettingsScreen(settings_screen, user_data);
    } else if (isPointWithinButton(settings_screen->save_button, mouse_pos) && settings_screen->save_button->is_selected) {
        saveUserData(user_data);
    }

    settings_screen->fps_button->is_selected = false;
    settings_screen->graphics_quality_button->is_selected = false;
    settings_screen->sensitivity_button->is_selected = false;
    settings_screen->fullscreen_button->is_selected = false;
    settings_screen->back_button->is_selected = false;
    settings_screen->reset_button->is_selected = false;
    settings_screen->save_button->is_selected = false;
}

void settingsScreenMouseDragged(SettingsScreen* settings_screen, const SDL_FPoint mouse_pos,
        UserData* user_data) {
    if (settings_screen->fps_button->is_selected) {
        float adjusted_x = mouse_pos.x - ((WINDOW_WIDTH - 800.0f) / 2.0f + 30.0f);
        adjusted_x = max(0.0f, min(730.0f, adjusted_x));
        const int slider_state = (int) round((double) adjusted_x / 730.0 * 2.0);
        int frame_rate_cap = 15;
        if (slider_state == 1) {
            frame_rate_cap = 30;
        } else if (slider_state == 2) {
            frame_rate_cap = 60;
        }
        user_data->frame_rate_cap = frame_rate_cap;
        const float slider_x = 730.0f * ((float) slider_state / 2.0f);
        settings_screen->fps_button->label->rect.x = slider_x + ((WINDOW_WIDTH - 800.0f) / 2.0f + 25.0f);
        setSettingsScreenLabels(settings_screen, user_data);
    }

    if (settings_screen->graphics_quality_button->is_selected) {
        float adjusted_x = mouse_pos.x - ((WINDOW_WIDTH - 800.0f) / 2.0f + 30.0f);
        adjusted_x = max(0.0f, min(730.0f, adjusted_x));
        user_data->graphics_quality = (int) round((double) adjusted_x / 730.0 * 2.0);
        const float slider_x = 730.0f * ((float) user_data->graphics_quality / 2.0f);
        settings_screen->graphics_quality_button->label->rect.x = slider_x + ((WINDOW_WIDTH - 800.0f) / 2.0f + 25.0f);
        setSettingsScreenLabels(settings_screen, user_data);
    }

    if (settings_screen->sensitivity_button->is_selected) {
        float adjusted_x = mouse_pos.x - ((WINDOW_WIDTH - 800.0f) / 2.0f + 30.0f);
        adjusted_x = max(0.0f, min(730.0f, adjusted_x));
        user_data->sensitivity = (int) (adjusted_x / 730.0f * 100.0f);
        const float slider_x = 730.0f * ((float) user_data->sensitivity / 100.0f);
        settings_screen->sensitivity_button->label->rect.x = slider_x + ((WINDOW_WIDTH - 800.0f) / 2.0f + 25.0f);
        setSettingsScreenLabels(settings_screen, user_data);
    }

    if (isPointWithinButton(settings_screen->fullscreen_button, mouse_pos)) {
        if (settings_screen->fullscreen_button->is_selected) {
            turnOnButton(settings_screen->fullscreen_button);
        }
    } else {
        turnOffButton(settings_screen->fullscreen_button);
    }

    if (isPointWithinButton(settings_screen->back_button, mouse_pos)) {
        if (settings_screen->back_button->is_selected) {
            turnOnButton(settings_screen->back_button);
        }
    } else {
        turnOffButton(settings_screen->back_button);
    }

    if (isPointWithinButton(settings_screen->reset_button, mouse_pos)) {
        if (settings_screen->reset_button->is_selected) {
            turnOnButton(settings_screen->reset_button);
        }
    } else {
        turnOffButton(settings_screen->reset_button);
    }

    if (isPointWithinButton(settings_screen->save_button, mouse_pos)) {
        if (settings_screen->save_button->is_selected) {
            turnOnButton(settings_screen->save_button);
        }
    } else {
        turnOffButton(settings_screen->save_button);
    }
}

static void setSettingsScreenBackground(SettingsScreen* settings_screen, SDL_Renderer* renderer) {
    settings_screen->left_background_texture =
        IMG_LoadTexture(renderer, "../game_data/textures/settings_screen/background_left.png");
    if (!settings_screen->left_background_texture) {
        SDL_Log("Failed to load image: %s", SDL_GetError());
    }
    settings_screen->right_background_texture =
        IMG_LoadTexture(renderer, "../game_data/textures/settings_screen/background_right.png");
    if (!settings_screen->right_background_texture) {
        SDL_Log("Failed to load image: %s", SDL_GetError());
    }
}

static void setSettingsScreenLabels(SettingsScreen* settings_screen, const UserData* user_data) {

    /*
     * consistent data
     */

    const LabelStyleData style_data = {
        .boarder_thickness = 10,
        .bevel_radius = 30,
        .boarder_color = {.r = 255, .g = 255, .b = 255, .a = 150}
    };
    LabelTextData text_data;
    const float line_spacing = 0.0f;
    const SDL_Color text_color = {.r = 255, .g = 255, .b = 255, .a = 150};

    /*
     * settings label
     */

    const char* text = "Settings";
    float font_size = 90.0f;
    float vertical_text_margin = 0.0f;
    float horizontal_text_margin = 0.0f;
    int vertical_text_alignment = ALIGN_CENTER;
    int horizontal_text_alignment = ALIGN_CENTER;
    initLabelTextData(&text_data, text, font_size, line_spacing, vertical_text_margin, horizontal_text_margin,
        vertical_text_alignment, horizontal_text_alignment, text_color);
    initLabel(&settings_screen->settings_label,
        (SDL_FRect) {.x = (WINDOW_WIDTH - 800.0f) / 2.0f, .y = 20.0f, .w = 800.0f, .h = 120.0f},
        (SDL_Color) {.r = 75, .g = 50, .b = 50, .a = 150}, style_data, text_data);

    /*
     * consistent data for all the following labels
     */

    font_size = 45.0f;
    vertical_text_margin = 15.0f;
    horizontal_text_margin = 25.0f;
    vertical_text_alignment = ALIGN_TOP;
    horizontal_text_alignment = ALIGN_LEFT;

    /*
     * fps label
     */

    char fps_label_text[19] = "Max Frame Rate: ";
    snprintf(fps_label_text, sizeof(fps_label_text), "%s%d",
        fps_label_text, user_data->frame_rate_cap);
    initLabelTextData(&text_data, fps_label_text, font_size, line_spacing, vertical_text_margin, horizontal_text_margin,
        vertical_text_alignment, horizontal_text_alignment, text_color);
    initLabel(&settings_screen->fps_label,
        (SDL_FRect) {.x = (WINDOW_WIDTH - 800.0f) / 2.0f, .y = 160.0f, .w = 800.0f, .h = 160.0f},
        (SDL_Color) {.r = 75, .g = 50, .b = 50, .a = 150}, style_data, text_data);

    /*
     * graphics quality label
     */

    switch (user_data->graphics_quality) {
        case GRAPHICS_QUALITY_LOW:
            text = "Floor/Ceiling Texture Resolution: LOW";
            break;
        case GRAPHICS_QUALITY_MEDIUM:
            text = "Floor/Ceiling Texture Resolution: MEDIUM";
            break;
        case GRAPHICS_QUALITY_HIGH:
            text = "Floor/Ceiling Texture Resolution: HIGH";
            break;
        default:
            break;
    }
    initLabelTextData(&text_data, text, font_size, line_spacing, vertical_text_margin, horizontal_text_margin,
        vertical_text_alignment, horizontal_text_alignment, text_color);
    initLabel(&settings_screen->graphics_quality_label,
        (SDL_FRect) {.x = (WINDOW_WIDTH - 800.0f) / 2.0f, .y = 340.0f, .w = 800.0f, .h = 160.0f},
        (SDL_Color) {.r = 75, .g = 50, .b = 50, .a = 150}, style_data, text_data);

    /*
     * sensitivity label
     */

    char sensitivity_label_text[17] = "Sensitivity: ";
    snprintf(sensitivity_label_text, sizeof(sensitivity_label_text), "%s%d",
        sensitivity_label_text, user_data->sensitivity);
    initLabelTextData(&text_data, sensitivity_label_text, font_size, line_spacing, vertical_text_margin,
        horizontal_text_margin, vertical_text_alignment, horizontal_text_alignment, text_color);
    initLabel(&settings_screen->sensitivity_label,
        (SDL_FRect) {.x = (WINDOW_WIDTH - 800.0f) / 2.0f, .y = 520.0f, .w = 800.0f, .h = 160.0f},
        (SDL_Color) {.r = 75, .g = 50, .b = 50, .a = 150}, style_data, text_data);
}

static void setSettingsScreenButtons(SettingsScreen* settings_screen, const UserData* user_data) {

    /*
     * consistent data
     */

    const LabelStyleData style_data = {
        .boarder_thickness = 10,
        .bevel_radius = 30,
        .boarder_color = {.r = 255, .g = 255, .b = 255, .a = 150}
    };
    LabelTextData text_data;
    const float font_size = 45.0f;
    const float line_spacing = 0.0f;
    const float vertical_text_margin = 0.0f;
    const float horizontal_text_margin = 0.0f;
    const int vertical_text_alignment = ALIGN_CENTER;
    const int horizontal_text_alignment = ALIGN_CENTER;
    const SDL_Color text_color = {.r = 255, .g = 255, .b = 255, .a = 150};
    const ButtonColorData color_data = {
        .boarder_click_color = (SDL_Color) {.r = 100, .g = 0, .b = 0, .a = 100},
        .boarder_idle_color = style_data.boarder_color,
        .text_click_color = (SDL_Color) {.r = 100, .g = 0, .b = 0, .a = 100},
        .text_idle_color = text_color,
        .fill_click_color = (SDL_Color) {.r = 75, .g = 50, .b = 50, .a = 150},
        .fill_idle_color = (SDL_Color) {.r = 75, .g = 50, .b = 50, .a = 150},
    };

    /*
     * fullscreen button
     */

    const char* text;
    if (user_data->fullscreen) {
        text = "Fullscreen: ON";
    } else {
        text = "Fullscreen: OFF";
    }
    initLabelTextData(&text_data, text, font_size, line_spacing, vertical_text_margin, horizontal_text_margin,
        vertical_text_alignment, horizontal_text_alignment, text_color);
    Label* fullscreen_label;
    initLabel(&fullscreen_label,
        (SDL_FRect) {.x = (WINDOW_WIDTH - 800.0f) / 2.0f, .y = 700.0f, .w = 800.0f, .h = 80.0f},
        (SDL_Color) {.r = 75, .g = 50, .b = 50, .a = 150}, style_data, text_data);
    initButton(&settings_screen->fullscreen_button, fullscreen_label, color_data);

    /*
     * back button
     */

    text = "Back";
    initLabelTextData(&text_data, text, font_size, line_spacing, vertical_text_margin, horizontal_text_margin,
        vertical_text_alignment, horizontal_text_alignment, text_color);
    Label* back_label;
    initLabel(&back_label,
        (SDL_FRect) {.x = (WINDOW_WIDTH - 800.0f) / 2.0f, .y = 800.0f, .w = 247, .h = 80.0f},
        (SDL_Color) {.r = 75, .g = 50, .b = 50, .a = 150}, style_data, text_data);
    initButton(&settings_screen->back_button, back_label, color_data);

    /*
     * reset button
     */

    text = "Reset";
    initLabelTextData(&text_data, text, font_size, line_spacing, vertical_text_margin, horizontal_text_margin,
        vertical_text_alignment, horizontal_text_alignment, text_color);
    Label* reset_label;
    initLabel(&reset_label,
        (SDL_FRect) {.x = (WINDOW_WIDTH - 800.0f) / 2.0f + 276.0f, .y = 800.0f, .w = 247, .h = 80.0f},
        (SDL_Color) {.r = 75, .g = 50, .b = 50, .a = 150}, style_data, text_data);
    initButton(&settings_screen->reset_button, reset_label, color_data);

    /*
     * save button
     */

    text = "Save";
    initLabelTextData(&text_data, text, font_size, line_spacing, vertical_text_margin, horizontal_text_margin,
        vertical_text_alignment, horizontal_text_alignment, text_color);
    Label* save_label;
    initLabel(&save_label,
        (SDL_FRect) {.x = (WINDOW_WIDTH - 800.0f) / 2.0f + 553.0f, .y = 800.0f, .w = 247, .h = 80.0f},
        (SDL_Color) {.r = 75, .g = 50, .b = 50, .a = 150}, style_data, text_data);
    initButton(&settings_screen->save_button, save_label, color_data);

    /*
     * consistent slider button data
     */

    const ButtonColorData slider_color_data = {
        .boarder_click_color = (SDL_Color) {.r = 100, .g = 100, .b = 100, .a = 255},
        .boarder_idle_color = {.r = 100, .g = 100, .b = 100, .a = 255},
        .text_click_color = (SDL_Color) {.r = 0, .g = 0, .b = 0, .a = 0},
        .text_idle_color = (SDL_Color) {.r = 0, .g = 0, .b = 0, .a = 0},
        .fill_click_color = (SDL_Color) {.r = 50, .g = 50, .b = 50, .a = 255},
        .fill_idle_color = (SDL_Color) {.r = 150, .g = 150, .b = 150, .a = 255}
    };
    const LabelStyleData slider_style_data = {
        .boarder_thickness = 2,
        .bevel_radius = 0,
        .boarder_color = {.r = 100, .g = 100, .b = 100, .a = 255}
    };
    initLabelTextData(&text_data, "", 0, 0, 0, 0,
        0, 0, (SDL_Color) {.r = 0, .g = 0, .b = 0, .a = 0});
    SDL_FRect default_slider_rect = {.x = (WINDOW_WIDTH - 800.0f) / 2.0f + 25.0f, .y = 235.0f, .w = 20.0f, .h = 60.0f};

    /*
     * fps slider button
     */

    int slider_index = 0;
    if (user_data->frame_rate_cap == 30) {
        slider_index = 1;
    } else if (user_data->frame_rate_cap == 60) {
        slider_index = 2;
    }
    SDL_FRect fps_rect = {
        .x = default_slider_rect.x + (float) slider_index * 365,
        .y = default_slider_rect.y,
        .w = default_slider_rect.w,
        .h = default_slider_rect.h
    };
    Label* fps_label;
    initLabel(&fps_label, fps_rect, (SDL_Color) {.r = 150, .g = 150, .b = 150, .a = 255}, slider_style_data, text_data);
    initButton(&settings_screen->fps_button, fps_label, slider_color_data);

    /*
     * graphics quality slider button
     */

    slider_index = 0;
    if (user_data->graphics_quality == GRAPHICS_QUALITY_MEDIUM) {
        slider_index = 1;
    } else if (user_data->graphics_quality == GRAPHICS_QUALITY_HIGH) {
        slider_index = 2;
    }
    SDL_FRect graphics_rect = {
        .x = default_slider_rect.x + (float) slider_index * 365.0f,
        .y = default_slider_rect.y + 180.0f,
        .w = default_slider_rect.w,
        .h = default_slider_rect.h
    };
    Label* graphics_label;
    initLabel(&graphics_label, graphics_rect,
        (SDL_Color) {.r = 150, .g = 150, .b = 150, .a = 255}, slider_style_data, text_data);
    initButton(&settings_screen->graphics_quality_button, graphics_label, slider_color_data);

    /*
     * sensitivity slider button
     */

    float slider_x = 730.0f * ((float) user_data->sensitivity / 100.0f);
    SDL_FRect sensitivity_rect = {
        .x = default_slider_rect.x + slider_x,
        .y = default_slider_rect.y + 360.0f,
        .w = default_slider_rect.w,
        .h = default_slider_rect.h
    };
    Label* sensitivity_label;
    initLabel(&sensitivity_label, sensitivity_rect,
        (SDL_Color) {.r = 150, .g = 150, .b = 150, .a = 255}, slider_style_data, text_data);
    initButton(&settings_screen->sensitivity_button, sensitivity_label, slider_color_data);
}

static void setSettingsScreenRects(SettingsScreen* settings_screen) {
    /*
     * fps slider rects
     */

    settings_screen->fps_slider_rects[0] =
        (SDL_FRect) {.x = (WINDOW_WIDTH - 800.0f) / 2.0f + 40.0f, .y = 260.0f, .w = 355.0f, .h = 10.0f};
    settings_screen->fps_slider_rects[1] =
        (SDL_FRect) {.x = (WINDOW_WIDTH - 800.0f) / 2.0f + 405.0f, .y = 260.0f, .w = 355.0f, .h = 10.0f};
    settings_screen->fps_slider_rects[2] =
        (SDL_FRect) {.x = (WINDOW_WIDTH - 800.0f) / 2.0f + 30.0f, .y = 240.0f, .w = 10.0f, .h = 50.0f};
    settings_screen->fps_slider_rects[3] =
        (SDL_FRect) {.x = (WINDOW_WIDTH - 800.0f) / 2.0f + 395.0f, .y = 240.0f, .w = 10.0f, .h = 50.0f};
    settings_screen->fps_slider_rects[4] =
        (SDL_FRect) {.x = (WINDOW_WIDTH - 800.0f) / 2.0f + 760.0f, .y = 240.0f, .w = 10.0f, .h = 50.0f};

    /*
     * graphics quality slider rects
     */

    settings_screen->graphics_quality_slider_rects[0] =
        (SDL_FRect) {.x = (WINDOW_WIDTH - 800.0f) / 2.0f + 40.0f, .y = 440.0f, .w = 355.0f, .h = 10.0f};
    settings_screen->graphics_quality_slider_rects[1] =
        (SDL_FRect) {.x = (WINDOW_WIDTH - 800.0f) / 2.0f + 405.0f, .y = 440.0f, .w = 355.0f, .h = 10.0f};
    settings_screen->graphics_quality_slider_rects[2] =
        (SDL_FRect) {.x = (WINDOW_WIDTH - 800.0f) / 2.0f + 30.0f, .y = 420.0f, .w = 10.0f, .h = 50.0f};
    settings_screen->graphics_quality_slider_rects[3] =
        (SDL_FRect) {.x = (WINDOW_WIDTH - 800.0f) / 2.0f + 395.0f, .y = 420.0f, .w = 10.0f, .h = 50.0f};
    settings_screen->graphics_quality_slider_rects[4] =
        (SDL_FRect) {.x = (WINDOW_WIDTH - 800.0f) / 2.0f + 760.0f, .y = 420.0f, .w = 10.0f, .h = 50.0f};

    /*
     * sensitivity slider rect
     */

    settings_screen->sensitivity_slider_rect =
        (SDL_FRect) {.x = (WINDOW_WIDTH - 800.0f) / 2.0f + 30.0f, .y = 620.0f, .w = 740.0f, .h = 10.0f};
}