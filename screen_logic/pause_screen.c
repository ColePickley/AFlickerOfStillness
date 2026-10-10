#include "pause_screen.h"

#include <stdio.h>
#include <tgmath.h>
#include <SDL3_image/SDL_image.h>

void initPauseScreen(PauseScreen** pause_screen, SDL_Renderer* renderer) {
    *pause_screen = malloc(sizeof(SettingsScreen));
    setPauseScreenBackground(*pause_screen, renderer);
    setPauseScreenLabels(*pause_screen);
    setPauseScreenButtons(*pause_screen);
    (*pause_screen)->background_texture_width = WINDOW_WIDTH / 3.0f;
    (*pause_screen)->background_texture_height = WINDOW_WIDTH / 3.0f / 16.0f * 9.0f;
    (*pause_screen)->last_paint_time = 0;
    (*pause_screen)->do_entrance_animation = false;
    (*pause_screen)->do_exit_animation = false;
    (*pause_screen)->animation_state = 0.0f;
}

void shuffleImages(PauseScreen* pause_screen) {
    const size_t n = sizeof(pause_screen->background_textures) / sizeof(pause_screen->background_textures[0]);
    if (n > 1) {
        for (size_t i = n - 1; i > 0; i--) {
            const size_t j = rand() % (i + 1);
            SDL_Texture* temp = pause_screen->background_textures[i];
            pause_screen->background_textures[i] = pause_screen->background_textures[j];
            pause_screen->background_textures[j] = temp;
        }
    }
}

void paintPauseScreen(PauseScreen* pause_screen, SDL_Renderer* renderer, bool* paint_prev_screen,
        bool* paint_pause_screen) {

    /*
     * checking for entrance animation
     */

    if (pause_screen->last_paint_time == 0) {
        pause_screen->last_paint_time = SDL_GetTicks();
        pause_screen->do_entrance_animation = true;
        shuffleImages(pause_screen);
    }

    for (int i = 0; i < 5; i++) {
        SDL_FRect bottom_rect = {
            .x = (float) (i / 5) * pause_screen->background_texture_width,
            .y = (float) (i % 5) * pause_screen->background_texture_height + pause_screen->animation_state,
            .w = pause_screen->background_texture_width,
            .h = pause_screen->background_texture_height
        };
        SDL_RenderTexture(renderer, pause_screen->background_textures[i], NULL, &bottom_rect);
        SDL_FRect top_rect = {
            .x = bottom_rect.x,
            .y = bottom_rect.y - pause_screen->background_texture_height * 5,
            .w = pause_screen->background_texture_width,
            .h = pause_screen->background_texture_height
        };
        SDL_RenderTexture(renderer, pause_screen->background_textures[i], NULL, &top_rect);
    }
    for (int i = 5; i < 10; i++) {
        SDL_FRect top_rect = {
            .x = (float) (i / 5) * pause_screen->background_texture_width,
            .y = (float) (i % 5) * pause_screen->background_texture_height - pause_screen->animation_state,
            .w = pause_screen->background_texture_width,
            .h = pause_screen->background_texture_height
        };
        SDL_RenderTexture(renderer, pause_screen->background_textures[i], NULL, &top_rect);
        SDL_FRect bottom_rect = {
            .x = top_rect.x,
            .y = top_rect.y + pause_screen->background_texture_height * 5,
            .w = pause_screen->background_texture_width,
            .h = pause_screen->background_texture_height
        };
        SDL_RenderTexture(renderer, pause_screen->background_textures[i], NULL, &bottom_rect);
    }
    for (int i = 10; i < 15; i++) {
        SDL_FRect bottom_rect = {
            .x = (float) (i / 5) * pause_screen->background_texture_width,
            .y = (float) (i % 5) * pause_screen->background_texture_height + pause_screen->animation_state,
            .w = pause_screen->background_texture_width,
            .h = pause_screen->background_texture_height
        };
        SDL_RenderTexture(renderer, pause_screen->background_textures[i], NULL, &bottom_rect);
        SDL_FRect top_rect = {
            .x = bottom_rect.x,
            .y = bottom_rect.y - pause_screen->background_texture_height * 5,
            .w = pause_screen->background_texture_width,
            .h = pause_screen->background_texture_height
        };
        SDL_RenderTexture(renderer, pause_screen->background_textures[i], NULL, &top_rect);
    }
    pause_screen->animation_state += 3.0f;
    if (pause_screen->animation_state > 1355.625f) {
        pause_screen->animation_state -= 1355.625f;
    }

    /*
     * entrance animation
     */
    //
    // if (settings_screen->left_background_rect.x < 0 && pause_screen->do_entrance_animation) {
    //     const uint64_t time_elapsed = SDL_GetTicks() - pause_screen->last_paint_time;
    //     settings_screen->left_background_rect.x += (float) time_elapsed * 1.7f;
    //     settings_screen->right_background_rect.x -= (float) time_elapsed * 1.7f;
    //     if (settings_screen->left_background_rect.x > 0) {
    //         settings_screen->left_background_rect.x = 0;
    //         settings_screen->right_background_rect.x = 0;
    //     }
    //     if (settings_screen->left_background_rect.x == 0) {
    //         pause_screen->do_entrance_animation = false;
    //     }
    // }
    //
    // /*
    //  * exit animation
    //  */
    //
    // if (settings_screen->left_background_rect.x > -2.0f * WINDOW_WIDTH / 3 && pause_screen->do_exit_animation) {
    //     const uint64_t time_elapsed = SDL_GetTicks() - pause_screen->last_paint_time;
    //     settings_screen->left_background_rect.x -= (float) time_elapsed * 1.7f;
    //     settings_screen->right_background_rect.x += (float) time_elapsed * 1.7f;
    //     if (settings_screen->left_background_rect.x < -2.0f * WINDOW_WIDTH / 3) {
    //         settings_screen->left_background_rect.x = -2.0f * WINDOW_WIDTH / 3;
    //         settings_screen->right_background_rect.x = 2.0f * WINDOW_WIDTH / 3;
    //     }
    //     if (settings_screen->left_background_rect.x == -2.0f * WINDOW_WIDTH / 3) {
    //         pause_screen->do_exit_animation = false;
    //         pause_screen->last_paint_time = 0;
    //         *paint_pause_screen = false;
    //     }
    // }
    //
    // /*
    //  * checking if an animation has ended
    //  */
    //
    // if (*paint_prev_screen && !pause_screen->do_entrance_animation && !pause_screen->do_exit_animation &&
    //         pause_screen->last_paint_time != 0) {
    //     *paint_prev_screen = false;
    // } else if (!*paint_prev_screen && pause_screen->do_exit_animation) {
    //     *paint_prev_screen = true;
    // }
    //
    // /*
    //  * drawing background
    //  */
    //
    // SDL_RenderTexture(renderer, settings_screen->left_background_texture, NULL,
    //     &settings_screen->left_background_rect);
    // SDL_RenderTexture(renderer, settings_screen->right_background_texture, NULL,
    //     &settings_screen->right_background_rect);
    //
    // /*
    //  * drawing settings
    //  */
    //
    // if (!pause_screen->do_entrance_animation && !pause_screen->do_exit_animation &&
    //         pause_screen->last_paint_time != 0) {
    //     paintLabel(renderer, settings_screen->settings_label);
    //     paintLabel(renderer, settings_screen->fps_label);
    //     paintLabel(renderer, settings_screen->graphics_quality_label);
    //     paintLabel(renderer, settings_screen->sensitivity_label);
    //
    //     SDL_SetRenderDrawColor(renderer, 255, 255, 255, 150);
    //     for (int i = 0; i < 5; i++) {
    //         SDL_RenderFillRect(renderer, &settings_screen->fps_slider_rects[i]);
    //         SDL_RenderFillRect(renderer, &settings_screen->graphics_quality_slider_rects[i]);
    //     }
    //     SDL_RenderFillRect(renderer, &settings_screen->sensitivity_slider_rect);
    //
    //     paintLabel(renderer, settings_screen->fps_button->label);
    //     paintLabel(renderer, settings_screen->graphics_quality_button->label);
    //     paintLabel(renderer, settings_screen->sensitivity_button->label);
    //
    //     paintLabel(renderer, settings_screen->fullscreen_button->label);
    //     paintLabel(renderer, settings_screen->back_button->label);
    //     paintLabel(renderer, settings_screen->reset_button->label);
    //     paintLabel(renderer, settings_screen->save_button->label);
    // }

    /*
     * recording the end time for the next animation cycle
     */

    if (pause_screen->last_paint_time != 0) {
        pause_screen->last_paint_time = SDL_GetTicks();
    }
}

void pauseScreenMousePressed(const PauseScreen* pause_screen, SDL_FPoint mouse_pos) {

}

void pauseScreenMouseReleased(PauseScreen* pause_screen, SDL_FPoint mouse_pos, bool* program_running,
        uint8_t* current_screen, uint8_t* prev_screen, UserData* user_data, SDL_Window* window) {

}

void pauseScreenMouseDragged(const PauseScreen* pause_screen, SDL_FPoint mouse_pos, UserData* user_data) {

}

void destroyPauseScreen(PauseScreen* pause_screen) {
    free(pause_screen);
    pause_screen = NULL;
}

static void setPauseScreenBackground(PauseScreen* pause_screen, SDL_Renderer* renderer) {
    for (int i = 0; i < 15; i++) {
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

static void setPauseScreenLabels(PauseScreen* pause_screen) {

}

static void setPauseScreenButtons(PauseScreen* pause_screen) {

}