#include "death_screen.h"

#include <stdio.h>
#include "../functions.h"

void initDeathScreen(DeathScreen** death_screen) {
    *death_screen = malloc(sizeof(DeathScreen));
    (*death_screen)->background_surface = SDL_CreateSurface(WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_PIXELFORMAT_RGBA8888);
    (*death_screen)->background_renderer = SDL_CreateSoftwareRenderer((*death_screen)->background_surface);
    SDL_SetRenderDrawBlendMode((*death_screen)->background_renderer, SDL_BLENDMODE_BLEND);
    (*death_screen)->last_points_1[0] = (SDL_FPoint) {.x = 0.0f, .y = 0.0f};
    (*death_screen)->last_points_1[1] = (SDL_FPoint) {.x = WINDOW_WIDTH, .y = 0.0f};
    (*death_screen)->last_points_1[2] = (SDL_FPoint) {.x = WINDOW_WIDTH, .y = WINDOW_HEIGHT};
    (*death_screen)->last_points_1[3] = (SDL_FPoint) {.x = 0.0f, .y = WINDOW_HEIGHT};
    (*death_screen)->last_points_2[0] = (SDL_FPoint) {.x = 0.0f, .y = 0.0f};
    (*death_screen)->last_points_2[1] = (SDL_FPoint) {.x = WINDOW_WIDTH, .y = 0.0f};
    (*death_screen)->last_points_2[2] = (SDL_FPoint) {.x = WINDOW_WIDTH, .y = WINDOW_HEIGHT};
    (*death_screen)->last_points_2[3] = (SDL_FPoint) {.x = 0.0f, .y = WINDOW_HEIGHT};
    (*death_screen)->do_entrance_animation = false;
    (*death_screen)->do_exit_animation = false;
    (*death_screen)->last_paint_time = 0;
    setDeathScreenLabel(*death_screen);
    setDeathScreenButtons(*death_screen);
}

void destroyDeathScreen(DeathScreen** death_screen) {
    if (*death_screen == NULL) return;
    SDL_DestroySurface((*death_screen)->background_surface);
    SDL_DestroyRenderer((*death_screen)->background_renderer);
    free(*death_screen);
    *death_screen = NULL;
}

int paintDeathScreen(DeathScreen* death_screen, SDL_Renderer* renderer) {
    int screen_change = -1;

    /*
     * checking for entrance animation
     */

    if (death_screen->last_paint_time == 0) {
        death_screen->last_paint_time = SDL_GetTicks();
        death_screen->do_entrance_animation = true;
        death_screen->animation_speed = 0.0001f;
        death_screen->sceen_alpha = 1.0f;
        SDL_SetRenderDrawBlendMode(death_screen->background_renderer, SDL_BLENDMODE_BLEND);
    }

    /*
     * Draw background
     */

    SDL_SetRenderDrawColor(renderer, 100, 100, 100, (int) death_screen->sceen_alpha);
    SDL_RenderFillRect(renderer, &(SDL_FRect) {.x = 0, .y = 0, .w = WINDOW_WIDTH, .h = WINDOW_HEIGHT});

    int iterations = 1;
    if (death_screen->do_entrance_animation) {
        iterations = 10;
        const uint64_t time_elapsed = SDL_GetTicks() - death_screen->last_paint_time;
        death_screen->sceen_alpha += (double) time_elapsed * death_screen->animation_speed;
        if (death_screen->sceen_alpha > 255) {
            death_screen->sceen_alpha = 255;
            death_screen->do_entrance_animation = false;
        }
    } else if (death_screen->do_exit_animation) {
        iterations = 10;
        const uint64_t time_elapsed = SDL_GetTicks() - death_screen->last_paint_time;
        death_screen->sceen_alpha -= (double) time_elapsed * death_screen->animation_speed;
        if (death_screen->sceen_alpha < 0) {
            death_screen->sceen_alpha = 0;
        }
    }
    if (!death_screen->do_exit_animation) {
        for (int i = 0; i < iterations; i++) {
            doPrimaryAnimation(death_screen);
        }
    } else {
        for (int i = 0; i < iterations; i++) {
            doExitAnimation(death_screen);
        }
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, death_screen->background_surface);
    SDL_RenderTexture(renderer, texture, NULL,
        &(SDL_FRect) {.x = 0.0f, .y = 0.0f, .w = WINDOW_WIDTH, .h = WINDOW_HEIGHT});
    SDL_DestroyTexture(texture);

    /*
     * Draw buttons and labels
     */

    if (!death_screen->do_entrance_animation && !death_screen->do_exit_animation) {
        paintLabel(renderer, death_screen->game_over_label);
        paintLabel(renderer, death_screen->last_checkpoint_button->label);
        paintLabel(renderer, death_screen->restart_level_button->label);
        paintLabel(renderer, death_screen->settings_button->label);
        paintLabel(renderer, death_screen->exit_button->label);
    }

    if (death_screen->do_exit_animation && death_screen->sceen_alpha == 0.0f) {
        death_screen->do_exit_animation = false;
        death_screen->last_paint_time = 0;
        screen_change = death_screen->next_screen;
    }

    if (death_screen->last_paint_time != 0) {
        death_screen->last_paint_time = SDL_GetTicks();
        while (death_screen->last_paint_time == 0) {
            death_screen->last_paint_time = SDL_GetTicks();
        }
    }

    return screen_change;
}

void doPrimaryAnimation(DeathScreen* death_screen) {
    const int iterations = 100;

    /*
     * Layer 1
     */

    const uint8_t color_component = random() % 200 + 50;
    SDL_SetRenderDrawColor(death_screen->background_renderer,
        color_component, color_component, color_component, 50);
    for (int i = 0; i < iterations; i++) {
        for (int pt_index = 0; pt_index < 4; pt_index++) {
            float direction = 1.0f;
            if (random() % 2 == 0) {
                direction = -direction;
            }
            SDL_FPoint new_point = {
                .x = death_screen->last_points_1[pt_index].x,
                .y = death_screen->last_points_1[pt_index].y
            };
            setLayer1NewPoint(&new_point, pt_index, direction);
            SDL_RenderLine(death_screen->background_renderer, new_point.x, new_point.y,
                death_screen->last_points_1[pt_index].x, death_screen->last_points_1[pt_index].y);
            death_screen->last_points_1[pt_index] = new_point;
        }
    }
    SDL_RenderPresent(death_screen->background_renderer);

    /*
     * layer 2
     */

    const uint8_t red = random() % 100 + 25;
    SDL_SetRenderDrawColor(death_screen->background_renderer, red, 0, 0, 50);
    for (int i = 0; i < iterations; i++) {
        for (int pt_index = 0; pt_index < 4; pt_index++) {
            float direction = 1.0f;
            if (random() % 2 == 0) {
                direction = -direction;
            }
            SDL_FPoint new_point = {
                .x = death_screen->last_points_2[pt_index].x,
                .y = death_screen->last_points_2[pt_index].y
            };
            setLayer2NewPoint(&new_point, pt_index, direction);
            SDL_RenderLine(death_screen->background_renderer, new_point.x, new_point.y,
                death_screen->last_points_2[pt_index].x, death_screen->last_points_2[pt_index].y);
            death_screen->last_points_2[pt_index] = new_point;
        }
    }
    SDL_RenderPresent(death_screen->background_renderer);
}

void doExitAnimation(DeathScreen* death_screen) {
    SDL_SetRenderDrawColor(death_screen->background_renderer, 0, 0, 0, 0);
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 4; j++) {
            float x_direction = 1.0f;
            float y_direction = 1.0f;
            if (random() % 2 == 0) {
                x_direction = -x_direction;
            }
            if (random() % 2 == 0) {
                y_direction = -y_direction;
            }
            SDL_FPoint new_point = {
                .x = death_screen->last_points_1[j].x,
                .y = death_screen->last_points_1[j].y
            };
            new_point.x = max(-50, min(WINDOW_WIDTH + 50, new_point.x + (float) (random() % 50) * x_direction));
            new_point.y = max(-50, min(WINDOW_HEIGHT + 50, new_point.y + (float) (random() % 50) * y_direction));
            SDL_RenderLine(death_screen->background_renderer, new_point.x, new_point.y,
                death_screen->last_points_1[j].x, death_screen->last_points_1[j].y);
            death_screen->last_points_1[j] = new_point;
        }
    }
    SDL_RenderPresent(death_screen->background_renderer);
}

/*
 * Event handling
 */

void deathScreenMousePressed(const DeathScreen* death_screen, const SDL_FPoint mouse_pos) {
    if (buttonPressCheck(death_screen->last_checkpoint_button, mouse_pos)) return;
    if (buttonPressCheck(death_screen->restart_level_button, mouse_pos)) return;
    if (buttonPressCheck(death_screen->settings_button, mouse_pos)) return;
    buttonPressCheck(death_screen->exit_button, mouse_pos);
}

int deathScreenMouseReleased(DeathScreen* death_screen, const SDL_FPoint mouse_pos, GameData** game_data) {
    if (buttonReleaseCheck(death_screen->last_checkpoint_button, mouse_pos)) {

    }
    if (buttonReleaseCheck(death_screen->restart_level_button, mouse_pos)) {

    }
    if (buttonReleaseCheck(death_screen->settings_button, mouse_pos)) {
        return SETTINGS_SCREEN;
    }
    if (buttonReleaseCheck(death_screen->exit_button, mouse_pos)) {
        death_screen->next_screen = START_SCREEN;
        death_screen->do_exit_animation = true;
        SDL_SetRenderDrawBlendMode(death_screen->background_renderer, SDL_BLENDMODE_NONE);
        destroyGameData(game_data);
        initGameData(game_data);
    }
    return -1;
}

void deathScreenMouseDragged(const DeathScreen* death_screen, const SDL_FPoint mouse_pos) {
    buttonMouseDragCheck(death_screen->last_checkpoint_button, mouse_pos);
    buttonMouseDragCheck(death_screen->restart_level_button, mouse_pos);
    buttonMouseDragCheck(death_screen->settings_button, mouse_pos);
    buttonMouseDragCheck(death_screen->exit_button, mouse_pos);
}

/*
 * Helper functions
 */

static void setLayer1NewPoint(SDL_FPoint* new_point, const uint8_t pt_index, const float direction) {
    switch (pt_index) {
        case 0:
            new_point->x = max(-50, min(WINDOW_WIDTH + 50, new_point->x + (float) (random() % 50) * direction));
            new_point->y = max(-50, min(WINDOW_HEIGHT + 50, new_point->y + (float) (random() % 50) * direction));
            break;
        case 1:
            new_point->x = max(-50, min(WINDOW_WIDTH + 50, new_point->x - (float) (random() % 50) * direction));
            new_point->y = max(-50, min(WINDOW_HEIGHT + 50, new_point->y + (float) (random() % 50) * direction));
            break;
        case 2:
            new_point->x = max(-50, min(WINDOW_WIDTH + 50, new_point->x - (float) (random() % 50) * direction));
            new_point->y = max(-50, min(WINDOW_HEIGHT + 50, new_point->y - (float) (random() % 50) * direction));
            break;
        case 3:
            new_point->x = max(-50, min(WINDOW_WIDTH + 50, new_point->x + (float) (random() % 50) * direction));
            new_point->y = max(-50, min(WINDOW_HEIGHT + 50, new_point->y - (float) (random() % 50) * direction));
            break;
        default:
            break;
    }
}

static void setLayer2NewPoint(SDL_FPoint* new_point, const uint8_t pt_index, const float direction) {
    const float slope1 = (float) WINDOW_HEIGHT / WINDOW_WIDTH;
    const float slope2 = (float) -WINDOW_HEIGHT / WINDOW_WIDTH;
    switch (pt_index) {
        case 0:
            new_point->x = max(0, min(WINDOW_WIDTH, new_point->x + (float) (random() % 50) * direction));
            new_point->y = max(slope1 * new_point->x - 50,
                min(slope1 * new_point->x + 50, new_point->y + (float) (random() % 50) * direction));
            break;
        case 1:
            new_point->x = max(0, min(WINDOW_WIDTH, new_point->x - (float) (random() % 50) * direction));
            new_point->y = max(slope2 * new_point->x + WINDOW_HEIGHT - 50,
                min(slope2 * new_point->x + WINDOW_HEIGHT + 50,
                new_point->y + (float) (random() % 50) * direction));
            break;
        case 2:
            new_point->x = max(0, min(WINDOW_WIDTH, new_point->x - (float) (random() % 50) * direction));
            new_point->y = max(slope1 * new_point->x - 50,
                min(slope1 * new_point->x + 50, new_point->y - (float) (random() % 50) * direction));
            break;
        case 3:
            new_point->x = max(0, min(WINDOW_WIDTH, new_point->x + (float) (random() % 50) * direction));
            new_point->y = max(slope2 * new_point->x + WINDOW_HEIGHT - 50,
                min(slope2 * new_point->x + WINDOW_HEIGHT + 50,
                new_point->y - (float) (random() % 50) * direction));
            break;
        default:
            break;
    }
}

static void setDeathScreenLabel(DeathScreen* death_screen) {
    const float x = (WINDOW_WIDTH - 800.0f) / 2.0f;
    const float y = 90.0f;
    const float width = 800.0f;
    const float height = 120.0f;
    const SDL_Color fill_color = {.r = 0, .g = 0, .b = 0, .a = 0};
    const LabelStyleData style_data = {
        .boarder_thickness = 10,
        .bevel_radius = 30,
        .boarder_color = {.r = 0, .g = 0, .b = 0, .a = 0}
    };
    const float font_size = 150.0f;
    const float line_spacing = 0.0f;
    const float vertical_text_margin = 0.0f; const float horizontal_text_margin = 0.0f;
    const int vertical_text_alignment = ALIGN_CENTER; const int horizontal_text_alignment = ALIGN_CENTER;
    const SDL_Color text_color = {.r = 255, .g = 255, .b = 255, .a = 100};
    LabelTextData resume_label_text_data;
    const char* play_label_text = "GAME OVER";
    initLabelTextData(&resume_label_text_data, play_label_text, font_size, line_spacing, vertical_text_margin,
        horizontal_text_margin, vertical_text_alignment, horizontal_text_alignment, text_color);
    initLabel(&death_screen->game_over_label, (SDL_FRect) {.x = x, .y = y, .w = width, .h = height}, fill_color,
        style_data, resume_label_text_data);
}

static void setDeathScreenButtons(DeathScreen* death_screen) {
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
    //Last checkpoint button
    LabelTextData last_checkpoint_label_text_data;
    const char* last_checkpoint_label_text = "Last Checkpoint";
    initLabelTextData(&last_checkpoint_label_text_data, last_checkpoint_label_text, font_size, line_spacing,
        vertical_text_margin, horizontal_text_margin, vertical_text_alignment, horizontal_text_alignment, text_color);
    Label* last_checkpoint_label = NULL;
    initLabel(&last_checkpoint_label, (SDL_FRect) {.x = x, .y = y + height + spacing, .w = width, .h = height},
        fill_color, style_data, last_checkpoint_label_text_data);
    initButton(&death_screen->last_checkpoint_button, last_checkpoint_label, color_data);
    //Restart level button
    LabelTextData restart_level_label_text_data;
    const char* restart_level_label_text = "Restart Level";
    initLabelTextData(&restart_level_label_text_data, restart_level_label_text, font_size, line_spacing,
        vertical_text_margin, horizontal_text_margin, vertical_text_alignment, horizontal_text_alignment, text_color);
    Label* restart_level_label = NULL;
    initLabel(&restart_level_label, (SDL_FRect) {.x = x, .y = y + (height + spacing) * 2, .w = width, .h = height},
        fill_color, style_data, restart_level_label_text_data);
    initButton(&death_screen->restart_level_button, restart_level_label, color_data);
    //Settings button
    LabelTextData settings_label_text_data;
    const char* settings_label_text = "Settings";
    initLabelTextData(&settings_label_text_data, settings_label_text, font_size, line_spacing, vertical_text_margin,
        horizontal_text_margin, vertical_text_alignment, horizontal_text_alignment, text_color);
    Label* settings_label = NULL;
    initLabel(&settings_label, (SDL_FRect) {.x = x, .y = y + (height + spacing) * 3, .w = width, .h = height},
        fill_color, style_data, settings_label_text_data);
    initButton(&death_screen->settings_button, settings_label, color_data);
    //Exit button
    LabelTextData exit_text_data;
    const char* exit_text = "Exit";
    initLabelTextData(&exit_text_data, exit_text, font_size, line_spacing, vertical_text_margin,
        horizontal_text_margin, vertical_text_alignment, horizontal_text_alignment, text_color);
    Label* exit_label = NULL;
    initLabel(&exit_label, (SDL_FRect) {.x = x, .y = y + (height + spacing) * 4, .w = width, .h = height},
        fill_color, style_data, exit_text_data);
    initButton(&death_screen->exit_button, exit_label, color_data);
}