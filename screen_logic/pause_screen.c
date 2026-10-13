#include "pause_screen.h"

#include "../functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <SDL3_image/SDL_image.h>

void initPauseScreen(PauseScreen** pause_screen, SDL_Renderer* renderer) {
    *pause_screen = malloc(sizeof(PauseScreen));
    setPauseScreenBackground(*pause_screen, renderer);
    setPauseScreenButtons(*pause_screen);
    (*pause_screen)->last_paint_time = 0;
    (*pause_screen)->do_entrance_animation = false;
    (*pause_screen)->do_exit_animation = false;
    (*pause_screen)->animation_state = 0.0f;
    (*pause_screen)->animation_speed = 0.0f;
}

void destroyPauseScreen(PauseScreen** pause_screen) {
    if (*pause_screen == NULL) return;
    for (int i = 0; i < 24; i++) {
        SDL_DestroyTexture((*pause_screen)->background_textures[i]);
    }
    free(*pause_screen);
    *pause_screen = NULL;
}

void shuffleBackgroundTextures(SDL_Texture* (*textures)[24]) {
    for (int i = 23; i > 0; i--) {
        const size_t j = random() % (i + 1);
        SDL_Texture* temp = (*textures)[i];
        (*textures)[i] = (*textures)[j];
        (*textures)[j] = temp;
    }
}

void shuffleTextureSet(SDL_Texture* (*textures)[12]) {
    for (int i = 11; i > 0; i--) {
        const size_t j = random() % (i + 1);
        SDL_Texture* temp = (*textures)[i];
        (*textures)[i] = (*textures)[j];
        (*textures)[j] = temp;
    }
}

int paintPauseScreen(PauseScreen* pause_screen, SDL_Renderer* renderer) {
    int screen_change = -1;

    /*
     * checking for entrance animation
     */

    if (pause_screen->last_paint_time == 0) {
        pause_screen->last_paint_time = SDL_GetTicks();
        pause_screen->do_entrance_animation = true;
        pause_screen->animation_speed = 2.0f;
        shuffleBackgroundTextures(&pause_screen->background_textures);
        for (int i = 0; i < 12; i++) {
            pause_screen->texture_set_1[i] = pause_screen->background_textures[i];
            pause_screen->texture_set_2[i] = pause_screen->background_textures[i + 12];
        }
    }

    /*
     * drawing background
     */

    for (int i = 0; i < 12; i++) {
        float animation_adjustment = 1.0f;
        if (i / 4 == 1) {
            animation_adjustment *= -1.0f;
        }
        SDL_FRect rect = {
            .x = (float) (int) ((float) i / 4.0f) * BACKGROUND_TEXTURE_WIDTH,
            .y = (float) (i % 4) * BACKGROUND_TEXTURE_HEIGHT + animation_adjustment * pause_screen->animation_state,
            .w = BACKGROUND_TEXTURE_WIDTH,
            .h = BACKGROUND_TEXTURE_HEIGHT
        };
        if (pause_screen->do_entrance_animation) {
            rect.y -= animation_adjustment * BACKGROUND_TEXTURE_HEIGHT * 4;
        }
        SDL_Texture* bottom_texture = pause_screen->texture_set_1[i];
        SDL_RenderTexture(renderer, bottom_texture, NULL, &rect);
        rect.y -= animation_adjustment * BACKGROUND_TEXTURE_HEIGHT * 4;
        SDL_Texture* top_texture = pause_screen->texture_set_2[i];
        SDL_RenderTexture(renderer, top_texture, NULL, &rect);
    }

    /*
     * painting buttons
     */

    if (!pause_screen->do_exit_animation && !pause_screen->do_entrance_animation) {
        paintLabel(renderer, pause_screen->resume_button->label);
        paintLabel(renderer, pause_screen->last_checkpoint_button->label);
        paintLabel(renderer, pause_screen->restart_level_button->label);
        paintLabel(renderer, pause_screen->settings_button->label);
        paintLabel(renderer, pause_screen->exit_button->label);
    }

    /*
     * adjusting animation
     */

    if (pause_screen->do_exit_animation) {
        pause_screen->animation_speed = 2.0f;
    }
    const uint64_t time_elapsed = SDL_GetTicks() - pause_screen->last_paint_time;
    pause_screen->animation_state += (float) time_elapsed * pause_screen->animation_speed;
    if (pause_screen->animation_state > 1084.5f && !pause_screen->do_exit_animation) {
        while (pause_screen->animation_state > 1084.5f) {
            pause_screen->animation_state -= 1084.5f;
        }
        if (pause_screen->do_entrance_animation) {
            pause_screen->do_entrance_animation = false;
            pause_screen->animation_speed = 0.15f;
        } else {
            for (int i = 0; i < 12; i++) {
                SDL_Texture* temp = pause_screen->texture_set_1[i];
                pause_screen->texture_set_1[i] = pause_screen->texture_set_2[i];
                pause_screen->texture_set_2[i] = temp;
            }
            shuffleTextureSet(&pause_screen->texture_set_2);
        }
    }

    if (pause_screen->animation_state >= 2169.0f) {
        pause_screen->do_exit_animation = false;
        pause_screen->last_paint_time = 0;
        screen_change = pause_screen->next_screen;
    }

    /*
     * recording the end time for the next animation cycle
     */

    if (pause_screen->last_paint_time != 0) {
        pause_screen->last_paint_time = SDL_GetTicks();
    }

    return screen_change;
}

/*
 * Event handling
 */

void pauseScreenMousePressed(const PauseScreen* pause_screen, const SDL_FPoint mouse_pos) {
    if (buttonPressCheck(pause_screen->resume_button, mouse_pos)) return;
    if (buttonPressCheck(pause_screen->last_checkpoint_button, mouse_pos)) return;
    if (buttonPressCheck(pause_screen->restart_level_button, mouse_pos)) return;
    if (buttonPressCheck(pause_screen->settings_button, mouse_pos)) return;
    buttonPressCheck(pause_screen->exit_button, mouse_pos);
}

int pauseScreenMouseReleased(PauseScreen* pause_screen, const SDL_FPoint mouse_pos, GameData** game_data) {
    if (buttonReleaseCheck(pause_screen->resume_button, mouse_pos)) {
        pause_screen->next_screen = GAME_SCREEN;
        pause_screen->do_exit_animation = true;
    }
    if (buttonReleaseCheck(pause_screen->last_checkpoint_button, mouse_pos)) {

    }
    if (buttonReleaseCheck(pause_screen->restart_level_button, mouse_pos)) {

    }
    if (buttonReleaseCheck(pause_screen->settings_button, mouse_pos)) {
        return SETTINGS_SCREEN;
    }
    if (buttonReleaseCheck(pause_screen->exit_button, mouse_pos)) {
        pause_screen->next_screen = START_SCREEN;
        pause_screen->do_exit_animation = true;
        destroyGameData(game_data);
        initGameData(game_data);
    }
    return -1;
}

void pauseScreenMouseDragged(const PauseScreen* pause_screen, const SDL_FPoint mouse_pos) {
    buttonMouseDragCheck(pause_screen->resume_button, mouse_pos);
    buttonMouseDragCheck(pause_screen->last_checkpoint_button, mouse_pos);
    buttonMouseDragCheck(pause_screen->restart_level_button, mouse_pos);
    buttonMouseDragCheck(pause_screen->settings_button, mouse_pos);
    buttonMouseDragCheck(pause_screen->exit_button, mouse_pos);
}

/*
 * Helper functions
 */

static void setPauseScreenBackground(PauseScreen* pause_screen, SDL_Renderer* renderer) {
    for (int i = 0; i < 24; i++) {
        char file_path[50] = "../game_data/textures/pause_screen/background_img";
        char file_type[5] = ".png";
        char full_path[56];
        snprintf(full_path, sizeof(full_path), "%s%d%s", file_path, i + 1, file_type);
        pause_screen->background_textures[i] = IMG_LoadTexture(renderer, full_path);
        if (!pause_screen->background_textures[i]) {
            SDL_Log("Failed to load image: %s", SDL_GetError());
        }
    }
}

static void setPauseScreenButtons(PauseScreen* pause_screen) {
    //consistent data
    const float x = (WINDOW_WIDTH - 800.0f) / 2.0f;
    const float y = 90.0f;
    const float width = 800.0f;
    const float height = 120.0f;
    const float spacing = 30.0f;
    const SDL_Color fill_color = {.r = 75, .g = 50, .b = 50, .a = 100};
    const LabelStyleData style_data = {
        .boarder_thickness = 10,
        .bevel_radius = 30,
        .boarder_color = {.r = 255, .g = 255, .b = 255, .a = 100}
    };
    const float font_size = 70.0f;
    const float line_spacing = 0.0f;
    const float vertical_text_margin = 0.0f; const float horizontal_text_margin = 0.0f;
    const int vertical_text_alignment = ALIGN_CENTER; const int horizontal_text_alignment = ALIGN_CENTER;
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
    //Resume button
    LabelTextData resume_label_text_data;
    const char* play_label_text = "Resume";
    initLabelTextData(&resume_label_text_data, play_label_text, font_size, line_spacing, vertical_text_margin,
        horizontal_text_margin, vertical_text_alignment, horizontal_text_alignment, text_color);
    Label* resume_label = NULL;
    initLabel(&resume_label, (SDL_FRect) {.x = x, .y = y, .w = width, .h = height}, fill_color, style_data,
        resume_label_text_data);
    initButton(&pause_screen->resume_button, resume_label, color_data);
    //Last checkpoint button
    LabelTextData last_checkpoint_label_text_data;
    const char* last_checkpoint_label_text = "Last Checkpoint";
    initLabelTextData(&last_checkpoint_label_text_data, last_checkpoint_label_text, font_size, line_spacing,
        vertical_text_margin, horizontal_text_margin, vertical_text_alignment, horizontal_text_alignment, text_color);
    Label* last_checkpoint_label = NULL;
    initLabel(&last_checkpoint_label, (SDL_FRect) {.x = x, .y = y + height + spacing, .w = width, .h = height},
        fill_color, style_data, last_checkpoint_label_text_data);
    initButton(&pause_screen->last_checkpoint_button, last_checkpoint_label, color_data);
    //Restart level button
    LabelTextData restart_level_label_text_data;
    const char* restart_level_label_text = "Restart Level";
    initLabelTextData(&restart_level_label_text_data, restart_level_label_text, font_size, line_spacing,
        vertical_text_margin, horizontal_text_margin, vertical_text_alignment, horizontal_text_alignment, text_color);
    Label* restart_level_label = NULL;
    initLabel(&restart_level_label, (SDL_FRect) {.x = x, .y = y + (height + spacing) * 2, .w = width, .h = height},
        fill_color, style_data, restart_level_label_text_data);
    initButton(&pause_screen->restart_level_button, restart_level_label, color_data);
    //Settings button
    LabelTextData settings_label_text_data;
    const char* settings_label_text = "Settings";
    initLabelTextData(&settings_label_text_data, settings_label_text, font_size, line_spacing, vertical_text_margin,
        horizontal_text_margin, vertical_text_alignment, horizontal_text_alignment, text_color);
    Label* settings_label = NULL;
    initLabel(&settings_label, (SDL_FRect) {.x = x, .y = y + (height + spacing) * 3, .w = width, .h = height},
        fill_color, style_data, settings_label_text_data);
    initButton(&pause_screen->settings_button, settings_label, color_data);
    //Exit button
    LabelTextData exit_text_data;
    const char* exit_text = "Exit";
    initLabelTextData(&exit_text_data, exit_text, font_size, line_spacing, vertical_text_margin,
        horizontal_text_margin, vertical_text_alignment, horizontal_text_alignment, text_color);
    Label* exit_label = NULL;
    initLabel(&exit_label, (SDL_FRect) {.x = x, .y = y + (height + spacing) * 4, .w = width, .h = height},
        fill_color, style_data, exit_text_data);
    initButton(&pause_screen->exit_button, exit_label, color_data);
}