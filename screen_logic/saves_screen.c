#include "saves_screen.h"

#include <SDL3_image/SDL_image.h>
#include "../constants.h"
#include "../file_handling/save_handling.h"
#include "../file_handling/map_reader.h"

void initSavesScreen(SavesScreen* saves_screen, SDL_Renderer* renderer) {
    setSavesScreenBackground(saves_screen, renderer);
    setSavesScreenLabels(saves_screen);
    setSavesScreenButtons(saves_screen);
    setSavesScreenDeleteSaveTexture(saves_screen, renderer);
}

void destroySavesScreen(const SavesScreen* saves_screen) {
    destroyLabel(saves_screen->left_panel_label);
    destroyLabel(saves_screen->right_panel_label);
    destroyLabel(saves_screen->select_save_label);
    destroyButton(saves_screen->back_button);
    destroyButton(saves_screen->play_button);
    for (int i = 0; i < 3; i++) {
        destroyButton(saves_screen->save_buttons[i]);
    }
    destroyButton(saves_screen->delete_save_button);
    SDL_DestroyTexture(saves_screen->background_texture);
    SDL_DestroyTexture(saves_screen->left_panel_texture);
    SDL_DestroyTexture(saves_screen->right_panel_texture);
}

void paintSavesScreen(const SavesScreen* saves_screen, SDL_Renderer* renderer) {
    SDL_RenderTexture(renderer, saves_screen->left_panel_texture, NULL, &saves_screen->left_panel_rect);
    SDL_RenderTexture(renderer, saves_screen->right_panel_texture, NULL, &saves_screen->right_panel_rect);
    paintLabel(renderer, saves_screen->left_panel_label);
    paintLabel(renderer, saves_screen->right_panel_label);
    const SDL_FRect background_rect = {
        .x = (float) (WINDOW_WIDTH - WINDOW_HEIGHT) / 2.0f,
        .y = 0.0f,
        .w = WINDOW_HEIGHT,
        .h = WINDOW_HEIGHT
    };
    SDL_RenderTexture(renderer, saves_screen->background_texture, NULL, &background_rect);
    paintLabel(renderer, saves_screen->select_save_label);
    paintLabel(renderer, saves_screen->saves_label);
    paintLabel(renderer, saves_screen->back_button->label);
    paintLabel(renderer, saves_screen->play_button->label);
    bool paint_delete_save_button = false;
    for (int i = 0; i < 3; i++) {
        paintLabel(renderer, saves_screen->save_buttons[i]->label);
        if (saves_screen->save_buttons[0]->is_on &&
                saves_screen->save_buttons[0]->label->text_data.horizontal_text_alignment == ALIGN_LEFT) {
            paint_delete_save_button = true;
        }
    }
    if (paint_delete_save_button) {
        paintLabel(renderer, saves_screen->delete_save_button->label);
        if (saves_screen->delete_save_button->is_on) {
            SDL_SetTextureAlphaMod(saves_screen->delete_save_texture, 255);
        } else {
            SDL_SetTextureAlphaMod(saves_screen->delete_save_texture, 100);
        }
        SDL_RenderTexture(renderer, saves_screen->delete_save_texture, NULL,
            &saves_screen->delete_save_button->label->rect);
    }
}

void savesScreenMousePressed(const SavesScreen* saves_screen, const SDL_FPoint mouse_pos) {
    bool save_selected = false;
    for (int i = 0; i < 3; i++) {
        if (saves_screen->save_buttons[i]->is_on) {
            save_selected = true;
            break;
        }
    }
    if (save_selected && buttonPressCheck(saves_screen->delete_save_button, mouse_pos)) return;

    bool button_released = false;
    const int clicked_save_button_index = buttonGroupPressCheck(3, saves_screen->save_buttons, mouse_pos);
    if (clicked_save_button_index != -1) {
        button_released = true;
        switch (clicked_save_button_index) {
            case 0:
                saves_screen->delete_save_button->label->rect.y = 255;
                break;
            case 1:
                saves_screen->delete_save_button->label->rect.y = 405;
                break;
            case 2:
                saves_screen->delete_save_button->label->rect.y = 555;
                break;
            default:
                break;
        }
    }

    if (buttonPressCheck(saves_screen->back_button, mouse_pos))
        button_released = true;
    if (buttonPressCheck(saves_screen->play_button, mouse_pos))
        button_released = true;
    if (!button_released) {
        for (int i = 0; i < 3; i++) {
            saves_screen->save_buttons[i]->is_selected = false;
        }
    }
}

void savesScreenMouseReleased(SavesScreen* saves_screen, const SDL_FPoint mouse_pos, GameData* game_data,
        uint8_t* current_screen, uint8_t* prev_screen) {
    bool button_released = false;
    for (int i = 0; i < 3; i++) {
        if (saves_screen->save_buttons[i]->is_selected) {
            game_data->save_index = i;
            button_released = true;
            break;
        }
    }

    if (buttonReleaseCheck(saves_screen->back_button, mouse_pos)) {
        for (int i = 0; i < 3; i++) {
            saves_screen->save_buttons[i]->is_selected = false;
            turnOffButton(saves_screen->save_buttons[i]);
        }
        *current_screen = *prev_screen;
        *prev_screen = SAVES_SCREEN;
        button_released = true;
    } else if (buttonReleaseCheck(saves_screen->play_button, mouse_pos)) {
        for (int i = 0; i < 3; i++) {
            saves_screen->save_buttons[i]->is_selected = false;
            turnOffButton(saves_screen->save_buttons[i]);
        }
        if (button_released) {
            *current_screen = GAME_SCREEN;
            *prev_screen = SAVES_SCREEN;
            game_data->level_index = getLevelIndex(game_data->save_index);
            readMap(game_data);
            loadSave(game_data);
            writeSave(game_data);
        } else {
            button_released = true;
        }
    } else if (buttonReleaseCheck(saves_screen->delete_save_button, mouse_pos)) {
        deleteSave(game_data->save_index);
        destroyButton(saves_screen->back_button);
        destroyButton(saves_screen->play_button);
        for (int i = 0; i < 3; i++) {
            destroyButton(saves_screen->save_buttons[i]);
        }
        destroyButton(saves_screen->delete_save_button);
        setSavesScreenButtons(saves_screen);
    }

    if (!button_released) {
        for (int i = 0; i < 3; i++) {
            turnOffButton(saves_screen->save_buttons[i]);
        }
    }
}

void savesScreenMouseDragged(const SavesScreen* saves_screen, const SDL_FPoint mouse_pos) {
    buttonMouseDragCheck(saves_screen->back_button, mouse_pos);
    buttonMouseDragCheck(saves_screen->play_button, mouse_pos);
    buttonMouseDragCheck(saves_screen->delete_save_button, mouse_pos);
}

static void setSavesScreenBackground(SavesScreen* saves_screen, SDL_Renderer* renderer) {
    saves_screen->background_texture = IMG_LoadTexture(renderer,
        "game_data/textures/saves_screen/background.png");
    if (!saves_screen->background_texture) {
        SDL_Log("Failed to load image: %s", SDL_GetError());
    }
    saves_screen->left_panel_texture = IMG_LoadTexture(renderer,
        "game_data/textures/saves_screen/left_panel.png");
    if (!saves_screen->left_panel_texture) {
        SDL_Log("Failed to load image: %s", SDL_GetError());
    }
    saves_screen->right_panel_texture = IMG_LoadTexture(renderer,
        "game_data/textures/saves_screen/right_panel.png");
    if (!saves_screen->right_panel_texture) {
        SDL_Log("Failed to load image: %s", SDL_GetError());
    }

    saves_screen->left_panel_rect = (SDL_FRect) {
        .x = ((float) (WINDOW_WIDTH - WINDOW_HEIGHT) / 2.0f - (float) WINDOW_HEIGHT) / 2.0f,
        .y = 0.0f,
        .w = (float) WINDOW_HEIGHT,
        .h = (float) WINDOW_HEIGHT
    };
    saves_screen->right_panel_rect = (SDL_FRect) {
        .x = (3.0f * (float) WINDOW_WIDTH - (float) WINDOW_HEIGHT) / 4.0f,
        .y = 0.0f,
        .w = (float) WINDOW_HEIGHT,
        .h = (float) WINDOW_HEIGHT
    };

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
        .boarder_thickness = 10.0f,
        .bevel_radius = 0.0f,
        .boarder_color = {.r = 0, .g = 0, .b = 0, .a = 100}
    };
    const LabelTextData text_data = {};
    initLabel(&saves_screen->left_panel_label, left_label_rect, (SDL_Color) {.r = 255, .g = 255, .b = 255, .a = 0},
        style_data, text_data);
    initLabel(&saves_screen->right_panel_label, right_label_rect, (SDL_Color) {.r = 255, .g = 255, .b = 255, .a = 0},
        style_data, text_data);
}

static void setSavesScreenLabels(SavesScreen* saves_screen) {
    /*
     * select save label
     */
    const LabelStyleData select_save_style_data = {
        .boarder_thickness = 10,
        .bevel_radius = 30,
        .boarder_color = {.r = 255, .g = 255, .b = 255, .a = 100}
    };
    LabelTextData select_save_text_data;
    const char* text = "Select Save:";
    const float font_size = 90.0f;
    const float line_spacing = 0.0f;
    const float vertical_text_margin = 0.0f;
    const float horizontal_text_margin = 0.0f;
    const int vertical_text_alignment = ALIGN_CENTER;
    const int horizontal_text_alignment = ALIGN_CENTER;
    const SDL_Color text_color = {.r = 255, .g = 255, .b = 255, .a = 100};
    initLabelTextData(&select_save_text_data, text, font_size, line_spacing, vertical_text_margin,
        horizontal_text_margin, vertical_text_alignment, horizontal_text_alignment, text_color);
    const SDL_FRect select_save_rect = (SDL_FRect) {
        .x = (WINDOW_WIDTH - 800.0f) / 2.0f,
        .y = (WINDOW_HEIGHT - 480.0f) / 2.0f - 150.0f,
        .w = 800.0f,
        .h = 120.0f
    };
    initLabel(&saves_screen->select_save_label, select_save_rect, (SDL_Color) {.r = 75, .g = 50, .b = 50, .a = 100},
        select_save_style_data, select_save_text_data);

    /*
     * saves label
     */

    const LabelStyleData saves_style_data = {
        .boarder_thickness = 10,
        .bevel_radius = 30,
        .boarder_color = {.r = 100, .g = 100, .b = 100, .a = 100}
    };
    LabelTextData saves_text_data;
    initLabelTextData(&saves_text_data, NULL, 0, 0, 0, 0,
        0, 0, (SDL_Color) {.r = 0, .g = 0, .b = 0, .a = 0});
    const SDL_FRect saves_rect = (SDL_FRect) {
        .x = (WINDOW_WIDTH - 800.0f) / 2.0f,
        .y = (WINDOW_HEIGHT - 480.0f) / 2.0f,
        .w = 800.0f,
        .h = 480.0f
    };
    initLabel(&saves_screen->saves_label, saves_rect, (SDL_Color) {.r = 200, .g = 200, .b = 200, .a = 100},
        saves_style_data, saves_text_data);
}

static void setSavesScreenButtons(SavesScreen* saves_screen) {
    /*
     * consistent data
     */

    const SDL_Color fill_color = {.r = 75, .g = 50, .b = 50, .a = 100};
    const LabelStyleData style_data = {
        .boarder_thickness = 10,
        .bevel_radius = 30,
        .boarder_color = {.r = 255, .g = 255, .b = 255, .a = 100}
    };
    const float font_size = 70.0f;
    const float line_spacing = 0.0f;
    const SDL_Color text_color = {.r = 255, .g = 255, .b = 255, .a = 100};

    const SDL_Color click_color = {.r = 100, .g = 0, .b = 0, .a = 100};
    const ButtonColorData color_data = {
        .boarder_click_color = click_color,
        .boarder_idle_color = style_data.boarder_color,
        .text_click_color = click_color,
        .text_idle_color = text_color,
        .fill_click_color = fill_color,
        .fill_idle_color = fill_color,
    };

    /*
     * back button
     */

    float vertical_text_margin = 0.0f; float horizontal_text_margin = 0.0f;
    int vertical_text_alignment = ALIGN_CENTER; int horizontal_text_alignment = ALIGN_CENTER;
    const float back_button_x = (WINDOW_WIDTH - 800.0f) / 2.0f;
    const float back_button_y = (WINDOW_HEIGHT - 480.0f) / 2.0f + 510.0f;
    const float back_button_width = 385.0f;
    const float back_button_height = 120.0f;
    LabelTextData back_button_text_data;
    const char* back_button_label_text = "Back";
    initLabelTextData(&back_button_text_data, back_button_label_text, font_size, line_spacing, vertical_text_margin,
        horizontal_text_margin, vertical_text_alignment, horizontal_text_alignment, text_color);
    SDL_FRect back_button_rect = {
        .x = back_button_x,
        .y = back_button_y,
        .w = back_button_width,
        .h = back_button_height
    };
    Label* back_button_label = NULL;
    initLabel(&back_button_label, back_button_rect, fill_color, style_data, back_button_text_data);
    initButton(&saves_screen->back_button, back_button_label, color_data);

    /*
     * play button
     */

    const float play_button_x = (WINDOW_WIDTH - 800.0f) / 2.0f + 415.0f;
    const float play_button_y = (WINDOW_HEIGHT - 480.0f) / 2.0f + 510.0f;
    const float play_button_width = 385.0f;
    const float play_button_height = 120.0f;
    LabelTextData play_button_text_data;
    const char* play_button_label_text = "Play";
    initLabelTextData(&play_button_text_data, play_button_label_text, font_size, line_spacing, vertical_text_margin,
        horizontal_text_margin, vertical_text_alignment, horizontal_text_alignment, text_color);
    SDL_FRect play_button_rect = {
        .x = play_button_x,
        .y = play_button_y,
        .w = play_button_width,
        .h = play_button_height
    };
    Label* play_button_label = NULL;
    initLabel(&play_button_label, play_button_rect, fill_color, style_data, play_button_text_data);
    initButton(&saves_screen->play_button, play_button_label, color_data);

    /*
     * save buttons
     */

    const float x = (WINDOW_WIDTH - 800.0f) / 2.0f + 30.0f;
    const float y = (WINDOW_HEIGHT - 480.0f) / 2.0f + 30.0f;
    const float width = 740.0f;
    const float height = 120.0f;
    // save buttons
    for (int i = 0; i < 3; i++) {
        const float spacing = 30.0f;
        LabelTextData save_text_data;
        char save_label_text[100];
        setSaveText(save_label_text, sizeof(save_label_text), i);
        if (strcmp(save_label_text, "EMPTY") == 0) {
            horizontal_text_alignment = ALIGN_CENTER;
            horizontal_text_margin = 0.0f;
        } else {
            horizontal_text_alignment = ALIGN_LEFT;
            horizontal_text_margin = 30.0f;
        }
        initLabelTextData(&save_text_data, save_label_text, font_size, line_spacing, vertical_text_margin,
            horizontal_text_margin, vertical_text_alignment, horizontal_text_alignment, text_color);
        Label* save_label = NULL;
        initLabel(&save_label, (SDL_FRect) {.x = x, .y = y + (height + spacing) * (float) i, .w = width, .h = height},
            fill_color, style_data, save_text_data);
        initButton(&saves_screen->save_buttons[i], save_label, color_data);
    }

    /*
     * delete save button
     */

    const LabelStyleData delete_save_style_data = {
        .boarder_thickness = 5,
        .bevel_radius = 20,
        .boarder_color = {.r = 0, .g = 0, .b = 0, .a = 100}
    };
    SDL_Color delete_save_fill_color = {.r = 100, .g = 100, .b = 100, .a = 100};
    LabelTextData delete_save_text_data;
    initLabelTextData(&delete_save_text_data, "", 0, 0, 0,
        0, 0, 0, (SDL_Color) {.r = 0, .g = 0, .b = 0, .a = 0});
    Label* delete_save_label = NULL;
    SDL_FRect delete_save_label_rect = {
        .x = x + width - height + 15,
        .y = 255,
        .w = height - 30,
        .h = height - 30
    };
    initLabel(&delete_save_label, delete_save_label_rect, delete_save_fill_color, delete_save_style_data,
        delete_save_text_data);
    ButtonColorData delete_save_color_data = {
        .fill_idle_color = delete_save_fill_color,
        .fill_click_color = {.r = 100, .g = 100, .b = 100, .a = 255},
        .boarder_idle_color = {.r = 0, .g = 0, .b = 0, .a = 100},
        .boarder_click_color = {.r = 0, .g = 0, .b = 0, .a = 255},
        .text_idle_color = {.r = 0, .g = 0, .b = 0, .a = 0},
        .text_click_color = {.r = 0, .g = 0, .b = 0, .a = 0}
    };
    initButton(&saves_screen->delete_save_button, delete_save_label, delete_save_color_data);
}

static void setSavesScreenDeleteSaveTexture(SavesScreen* saves_screen, SDL_Renderer* renderer) {
    saves_screen->delete_save_texture = IMG_LoadTexture(renderer,
        "game_data/textures/saves_screen/trash_can.png");
    if (!saves_screen->delete_save_texture) {
        SDL_Log("Failed to load image: %s", SDL_GetError());
    }
    SDL_SetTextureBlendMode(saves_screen->delete_save_texture, SDL_BLENDMODE_BLEND);
}