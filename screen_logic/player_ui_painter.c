#include "player_ui_painter.h"

#include <stdio.h>
#include <stdlib.h>
#include "painter.h"
#include "../functions.h"
#include "SDL3_image/SDL_image.h"

void initPlayerUIPainter(PlayerUIPainter** ui_painter, GameData* game_data) {
    *ui_painter = malloc(sizeof(PlayerUIPainter));
    setPlayerUIPainterLabels(*ui_painter);
    setHotbarSurfaces(*ui_painter);
    setWeaponSurfaces(*ui_painter);
}

void setPlayerUIPainterTextures(PlayerUIPainter* ui_painter, SDL_Renderer* renderer) {
    //ammo texture
    SDL_Texture* ammo_texture = SDL_CreateTextureFromSurface(renderer, ui_painter->ammo_surface);
    if (!ammo_texture) {
        SDL_Log("Failed to create texture: %s", SDL_GetError());
        return;
    }
    SDL_SetTextureScaleMode(ammo_texture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureBlendMode(ammo_texture, SDL_BLENDMODE_BLEND);
    ui_painter->ammo_texture = ammo_texture;
    //mag texture
    SDL_Texture* mag_texture = SDL_CreateTextureFromSurface(renderer, ui_painter->mag_surface);
    if (!mag_texture) {
        SDL_Log("Failed to create texture: %s", SDL_GetError());
        return;
    }
    SDL_SetTextureScaleMode(mag_texture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureBlendMode(mag_texture, SDL_BLENDMODE_BLEND);
    ui_painter->mag_texture = mag_texture;
    //weapon textures
    for (int i = 0; i < ui_painter->num_weapon_textures; i++) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, ui_painter->weapon_surfaces[i]);
        if (!texture) {
            SDL_Log("Failed to create texture: %s", SDL_GetError());
            return;
        }
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        ui_painter->weapon_textures[i] = texture;
    }
}

void destroyPlayerUIPainter(PlayerUIPainter** ui_painter) {
    for (int i = 0; i < (*ui_painter)->num_weapon_textures; i++) {
        SDL_DestroyTexture((*ui_painter)->weapon_textures[i]);
    }
    SDL_DestroyTexture((*ui_painter)->ammo_texture);
    SDL_DestroyTexture((*ui_painter)->mag_texture);
    for (int i = 0; i < MAX_NUM_KEYS; i++) {
        if ((*ui_painter)->key_textures[i] != NULL) {
            SDL_DestroyTexture((*ui_painter)->key_textures[i]);
        }
    }
    for (int i = 0; i < 4; i++) {
        destroyLabel(&(*ui_painter)->hotbar_labels[i]);
    }
    free(*ui_painter);
    *ui_painter = NULL;
}

void paintPlayerUI(PlayerUIPainter* ui_painter, SDL_Renderer* renderer, const Player* player,
        SDL_Texture* const entity_textures[MAX_NUM_MAP_TEXTURES]) {
    paintHotbar(ui_painter, renderer, player, entity_textures);
    paintKeys(ui_painter, renderer, player, entity_textures);
    // cross-air
    const SDL_FRect rect1 = {.x = WINDOW_WIDTH / 2.0f - 4.0f, .y = WINDOW_HEIGHT / 2.0f - 18.0f, .w = 8.0f, .h = 36.0f};
    const SDL_FRect rect2 = {.x = WINDOW_WIDTH / 2.0f - 18.0f, .y = WINDOW_HEIGHT / 2.0f - 4.0f, .w = 36.0f, .h = 8.0f};
    const SDL_FRect rect3 = {.x = WINDOW_WIDTH / 2.0f - 2.0f, .y = WINDOW_HEIGHT / 2.0f - 16.0f, .w = 4.0f, .h = 32.0f};
    const SDL_FRect rect4 = {.x = WINDOW_WIDTH / 2.0f - 16.0f, .y = WINDOW_HEIGHT / 2.0f - 2.0f, .w = 32.0f, .h = 4.0f};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
    SDL_RenderFillRect(renderer, &rect1);
    SDL_RenderFillRect(renderer, &rect2);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);
    SDL_RenderFillRect(renderer, &rect3);
    SDL_RenderFillRect(renderer, &rect4);
}

/*
 *----------------------------------------------------------------------------------------------------------------------
 * Helper functions
 *----------------------------------------------------------------------------------------------------------------------
 */

static void paintHotbar(const PlayerUIPainter* ui_painter, SDL_Renderer* renderer, const Player* player,
        SDL_Texture* const entity_textures[MAX_NUM_MAP_TEXTURES]) {
    const SDL_Color background_color = {.r = 0, .g = 0, .b = 0, .a = 100};

    /*
     * POV Weapon
     */

    const SDL_FRect pov_weapon_rect = {
        .x = WINDOW_WIDTH - 700.0f,
        .y = WINDOW_HEIGHT / 4.0f,
        .w = 700.0f,
        .h = WINDOW_HEIGHT * 3.0f / 4.0f
    };
    if (player->primary_selected && player->primary_weapon != NULL) {
        SDL_RenderTexture(renderer, ui_painter->weapon_textures[player->primary_weapon->weapon_id], NULL,
            &pov_weapon_rect);
    } else if (!player->primary_selected && player->secondary_weapon != NULL) {
        SDL_RenderTexture(renderer, ui_painter->weapon_textures[player->secondary_weapon->weapon_id], NULL,
            &pov_weapon_rect);
    }

    /*
     * Ammo and Mags
     */

    // background
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);
    SDL_RenderFillRect(renderer, &(SDL_FRect) {.x = 0.0f, .y = WINDOW_HEIGHT - 200.0f, .w = 400.0f, .h = 200.0f});
    const SDL_FPoint ammo_points[3] = {
        (SDL_FPoint) {.x = 400.0f, .y = WINDOW_HEIGHT},
        (SDL_FPoint) {.x = 400.0f, .y = WINDOW_HEIGHT - 200.0f},
        (SDL_FPoint) {.x = 600.0f, .y = WINDOW_HEIGHT}
    };
    paintPolygon(renderer, 3, ammo_points, background_color);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderLine(renderer, 400.0f, WINDOW_HEIGHT - 200.0f, 600.0f, WINDOW_HEIGHT);
    SDL_RenderLine(renderer, 399.0f, WINDOW_HEIGHT - 200.0f, 599.0f, WINDOW_HEIGHT);
    SDL_RenderFillRect(renderer, &(SDL_FRect) {.x = 0.0f, .y = WINDOW_HEIGHT - 201.0f, .w = 400.0f, .h = 2.0f});
    SDL_RenderFillRect(renderer, &(SDL_FRect) {.x = 0.0f, .y = WINDOW_HEIGHT - 101.0f, .w = 500.0f, .h = 2.0f});

    // textures
    const SDL_FRect ammo_rect = {
        .x = 5.0f,
        .y = WINDOW_HEIGHT - 195.0f,
        .w = 90.0f,
        .h = 90.0f,
    };
    SDL_RenderTexture(renderer, ui_painter->ammo_texture, NULL, &ammo_rect);
    const SDL_FRect mag_rect = {
        .x = 80.0f,
        .y = WINDOW_HEIGHT - 95.0f,
        .w = 90.0f,
        .h = 90.0f,
    };
    SDL_RenderTexture(renderer, ui_painter->mag_texture, NULL, &mag_rect);

    // text
    char ammo_text[6] = {'\0'};
    char mag_text[6] = {'\0'};
    if (player->primary_weapon == NULL) {
        snprintf(ammo_text, sizeof(ammo_text), "%02d/%02d", 0, 0);
        snprintf(mag_text, sizeof(mag_text), "%02d/%02d", 0, 0);
    } else {
        snprintf(ammo_text, sizeof(ammo_text), "%02d/%02d", player->primary_weapon->ammo_count,
            player->primary_weapon->max_ammo_count);
        snprintf(mag_text, sizeof(mag_text), "%02d/%02d", player->primary_weapon->mag_count,
            player->primary_weapon->max_mag_count);
    }
    paintText(renderer, ammo_text, (SDL_FPoint) {.x = 110.0f, .y = WINDOW_HEIGHT - 195.0f}, 90.0f,
            (SDL_Color) {.r = 255, .g = 255, .b = 255, .a = 255}, FONT_PIXELOID);
    paintText(renderer, mag_text, (SDL_FPoint) {.x = 185.0f, .y = WINDOW_HEIGHT - 95.0f}, 90.0f,
            (SDL_Color) {.r = 255, .g = 255, .b = 255, .a = 255}, FONT_PIXELOID);

    /*
     * Weapon textures
     */

    // background
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);
    SDL_RenderFillRect(renderer, &(SDL_FRect) {.x = 0.0f, .y = 0.0f, .w = 160.0f, .h = 160.0f});
    SDL_RenderFillRect(renderer, &(SDL_FRect) {.x = 0.0f, .y = 160.0f, .w = 90.0f, .h = 90.0f});
    const SDL_FPoint points_1[3] = {
        (SDL_FPoint) {.x = 160.0f, .y = 0.0f},
        (SDL_FPoint) {.x = 160.0f, .y = 160.0f},
        (SDL_FPoint) {.x = 240.0f, .y = 0.0f}
    };
    paintPolygon(renderer, 3, points_1, background_color);
    const SDL_FPoint points_2[3] = {
        (SDL_FPoint) {.x = 90.0f, .y = 160.0f},
        (SDL_FPoint) {.x = 90.0f, .y = 250.0f},
        (SDL_FPoint) {.x = 135.0f, .y = 160.0f},
    };
    paintPolygon(renderer, 3, points_2, background_color);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderLine(renderer, 240.0f, 0.0f, 160.0f, 160.0f);
    SDL_RenderLine(renderer, 239.0f, 0.0f, 159.0f, 160.0f);
    SDL_RenderFillRect(renderer, &(SDL_FRect) {.x = 0.0f, .y = 159.0f, .w = 160.0f, .h = 2.0f});
    SDL_RenderLine(renderer, 135.0f, 160.0f, 90.0f, 250.0f);
    SDL_RenderLine(renderer, 134.0f, 160.0f, 89.0f, 250.0f);
    SDL_RenderFillRect(renderer, &(SDL_FRect) {.x = 0.0f, .y = 249.0f, .w = 90.0f, .h = 2.0f});

    // textures
    const SDL_FRect selected_weapon_rect = {
        .x = 5.0f,
        .y = 5.0f,
        .w = 150.0f,
        .h = 150.0f,
    };
    const SDL_FRect deselected_weapon_rect = {
        .x = 5.0f,
        .y = 165.0f,
        .w = 80.0f,
        .h = 80.0f,
    };
    if (player->primary_weapon != NULL) {
        if (player->primary_selected) {
            SDL_RenderTexture(renderer, entity_textures[player->primary_weapon->entity_id], NULL,
                &selected_weapon_rect);
        } else {
            SDL_RenderTexture(renderer, entity_textures[player->primary_weapon->entity_id], NULL,
                &deselected_weapon_rect);
        }
    }
    if (player->secondary_weapon != NULL) {
        if (player->primary_selected) {
            SDL_RenderTexture(renderer, entity_textures[player->secondary_weapon->entity_id], NULL,
                &deselected_weapon_rect);
        } else {
            SDL_RenderTexture(renderer, entity_textures[player->secondary_weapon->entity_id], NULL,
                &selected_weapon_rect);
        }
    }

    // text
    const char primary_text[2] = {'1', '\0'};
    const char secondary_text[2] = {'2', '\0'};
    if (player->primary_selected) {
        paintText(renderer, primary_text, (SDL_FPoint) {.x = 230.0f, .y = 60.0f}, 40.0f,
            (SDL_Color) {.r = 0, .g = 0, .b = 0, .a = 255}, FONT_PIXELOID);
        paintText(renderer, primary_text, (SDL_FPoint) {.x = 230.0f, .y = 60.0f}, 36.0f,
            (SDL_Color) {.r = 255, .g = 255, .b = 255, .a = 255}, FONT_PIXELOID);
        paintText(renderer, secondary_text, (SDL_FPoint) {.x = 125.0f, .y = 193.0f}, 22.5f,
            (SDL_Color) {.r = 0, .g = 0, .b = 0, .a = 255}, FONT_PIXELOID);
        paintText(renderer, secondary_text, (SDL_FPoint) {.x = 125.0f, .y = 193.0f}, 18.5f,
            (SDL_Color) {.r = 255, .g = 255, .b = 255, .a = 255}, FONT_PIXELOID);
    } else {
        paintText(renderer, secondary_text, (SDL_FPoint) {.x = 230.0f, .y = 60.0f}, 40.0f,
            (SDL_Color) {.r = 0, .g = 0, .b = 0, .a = 255}, FONT_PIXELOID);
        paintText(renderer, secondary_text, (SDL_FPoint) {.x = 230.0f, .y = 60.0f}, 36.0f,
            (SDL_Color) {.r = 255, .g = 255, .b = 255, .a = 255}, FONT_PIXELOID);
        paintText(renderer, primary_text, (SDL_FPoint) {.x = 125.0f, .y = 193.0f}, 22.5f,
            (SDL_Color) {.r = 0, .g = 0, .b = 0, .a = 255}, FONT_PIXELOID);
        paintText(renderer, primary_text, (SDL_FPoint) {.x = 125.0f, .y = 193.0f}, 18.5f,
            (SDL_Color) {.r = 255, .g = 255, .b = 255, .a = 255}, FONT_PIXELOID);
    }

    /*
     * Health bar
     */

    //background
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);
    SDL_RenderFillRect(renderer, &(SDL_FRect) {.x = WINDOW_WIDTH - 700.0f, .y = 0.0f, .w = 700.0f, .h = 100.0f});
    const SDL_FPoint health_points_1[3] = {
        (SDL_FPoint) {.x = WINDOW_WIDTH - 800.0f, .y = 0.0f},
        (SDL_FPoint) {.x = WINDOW_WIDTH - 700.0f, .y = 0.0f},
        (SDL_FPoint) {.x = WINDOW_WIDTH - 700.0f, .y = 100.0f}
    };
    paintPolygon(renderer, 3, health_points_1, background_color);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderLine(renderer, WINDOW_WIDTH - 800.0f, 0.0f, WINDOW_WIDTH - 700.0f, 100.0f);
    SDL_RenderLine(renderer, WINDOW_WIDTH - 801.0f, 0.0f, WINDOW_WIDTH - 701.0f, 100.0f);
    SDL_RenderFillRect(renderer, &(SDL_FRect) {.x = WINDOW_WIDTH - 700.0f, .y = 100.0f, .w = 700.0f, .h = 2.0f});

    // outline
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &(SDL_FRect) {.x = WINDOW_WIDTH - 690.0f, .y = 10.0f, .w = 690.0f, .h = 80.0f});
    const SDL_FPoint health_points_2[3] = {
        (SDL_FPoint) {.x = WINDOW_WIDTH - 770.0f, .y = 10.0f},
        (SDL_FPoint) {.x = WINDOW_WIDTH - 690.0f, .y = 10.0f},
        (SDL_FPoint) {.x = WINDOW_WIDTH - 690.0f, .y = 90.0f}
    };
    paintPolygon(renderer, 3, health_points_2, (SDL_Color) {.r = 255, .g = 255, .b = 255, .a = 255});

    // health bar
    const float health_bar_width = 755.0f;
    const int health_bar_x = WINDOW_WIDTH - (int) health_bar_width;
    const int health_bar_start_x = health_bar_x +
        (int) ((1.0f - (float) player->health / 100.0f) * (float) health_bar_width);
    for (int i = health_bar_start_x; i < WINDOW_WIDTH; i++) {
        const uint8_t r = (uint8_t) ((float) (i - health_bar_x) / health_bar_width * 255.0f);
        const uint8_t g = (uint8_t) ((1.0f - (float) (i - health_bar_x) / health_bar_width) * 255.0f);
        SDL_SetRenderDrawColor(renderer, min(255, r), min(255, g), 0, 255);
        const float y2 = 84.0f - max(0.0f, health_bar_width - (float) i + health_bar_x - 686.0f);
        SDL_RenderLine(renderer, (float) i, 15.0f, (float) i, y2);
    }

    /*
     * Sprint bar
     */

    const Circle inner_circle = {
        .x = WINDOW_WIDTH - 110.0f,
        .y = WINDOW_HEIGHT - 110.0f,
        .inner_radius = 0.0f,
        .outer_radius = 198.0f
    };
    paintFilledCircle(renderer, inner_circle, background_color);
    const Circle outer_circle = {
        .x = WINDOW_WIDTH - 110.0f,
        .y = WINDOW_HEIGHT - 110.0f,
        .inner_radius = 199.0f,
        .outer_radius = 200.0f
    };
    paintFilledCircle(renderer, outer_circle, (SDL_Color) {.r = 255, .g = 255, .b = 255, .a = 255});
    const Circle sprint_meter_circle_1 = {
        .x = WINDOW_WIDTH - 110.0f,
        .y = WINDOW_HEIGHT - 110.0f,
        .inner_radius = 95.0f,
        .outer_radius = 155.0f
    };
    const SemiCircle sprint_meter_1 = {
        .c = sprint_meter_circle_1,
        .start_angle = 148.0f,
        .end_angle = 302.0f,
        .num_segments = 32
    };
    paintSemiCircle(renderer, sprint_meter_1, (SDL_Color) {.r = 255, .g = 255, .b = 255, .a = 255});
    const Circle sprint_meter_circle_2 = {
        .x = WINDOW_WIDTH - 110.0f,
        .y = WINDOW_HEIGHT - 110.0f,
        .inner_radius = 100.0f,
        .outer_radius = 150.0f
    };
    const SemiCircle sprint_meter_2 = {
        .c = sprint_meter_circle_2,
        .start_angle = 150.0f,
        .end_angle = 300.0f - (float) (MAX_SPRINT_TIME - player->sprint_time) / MAX_SPRINT_TIME * 150.0f,
        .num_segments = 32
    };
    paintSemiCircle(renderer, sprint_meter_2, (SDL_Color) {.r = 0, .g = 0, .b = 255, .a = 255});
}

static void paintKeys(PlayerUIPainter* ui_painter, SDL_Renderer* renderer, const Player* player,
        SDL_Texture* const entity_textures[MAX_NUM_MAP_TEXTURES]) {
    for (int i = 0; i < player->num_keys; i++) {
        SDL_FRect dst_rect = {
            .x = 5.0f,
            .y = (float) i * 75.0f + 255.0f,
            .w = 75.0f,
            .h = 75.0f
        };
        SDL_RenderTexture(renderer, entity_textures[player->keys[i]->entity_id], NULL, &dst_rect);
    }
}

static void setPlayerUIPainterLabels(PlayerUIPainter* ui_painter) {
    // consistent data
    const SDL_Color fill_color = {.r = 200, .g = 200, .b = 200, .a = 255};
    const SDL_Color border_color = {.r = 50, .g = 50, .b = 50, .a = 255};
    const LabelStyleData style_data= {.bevel_radius = 0.0f, .border_thickness = 5.0f, .border_color = border_color};

    /*
     * Ammo and Mag Labels
     */

    LabelTextData text_data1;
    initLabelTextData(&text_data1, "00/00", FONT_PIXELOID, 80.0f, 0.0f,
        0.0f,10.0f, ALIGN_CENTER, ALIGN_RIGHT,
        border_color);

    const SDL_FRect rect1 = {
        .x = 10.0f,
        .y = WINDOW_HEIGHT - HOTBAR_HEIGHT + 10.0f,
        .w = 400.0f,
        .h = HOTBAR_HEIGHT - 20,
    };
    initLabel(&ui_painter->hotbar_labels[0], rect1, fill_color, style_data, text_data1);
    const SDL_FRect rect2 = {
        .x = 420.0f,
        .y = WINDOW_HEIGHT - HOTBAR_HEIGHT + 10.0f,
        .w = 400.0f,
        .h = HOTBAR_HEIGHT - 20,
    };
    initLabel(&ui_painter->hotbar_labels[1], rect2, fill_color, style_data, text_data1);

    /*
     * Weapon Labels
     */

    LabelTextData text_data2;
    initDefaultLabelTextData(&text_data2);

    const SDL_FRect rect3 = {
        .x = 830.0f,
        .y = WINDOW_HEIGHT - HOTBAR_HEIGHT + 10.0f,
        .w = 250.0f,
        .h = HOTBAR_HEIGHT - 20,
    };
    initLabel(&ui_painter->hotbar_labels[2], rect3, fill_color, style_data, text_data2);
    const SDL_FRect rect4 = {
        .x = 1090.0f,
        .y = WINDOW_HEIGHT - HOTBAR_HEIGHT + 10.0f,
        .w = 250.0f,
        .h = HOTBAR_HEIGHT - 20,
    };
    initLabel(&ui_painter->hotbar_labels[3], rect4, fill_color, style_data, text_data2);

    /*
     * Health bar label
     */

    const SDL_FRect rect5 = {
        .x = 1350.0f,
        .y = WINDOW_HEIGHT - HOTBAR_HEIGHT + 10.0f,
        .w = WINDOW_WIDTH - 1360.0f,
        .h = HOTBAR_HEIGHT - 20.0f,
    };
    initLabel(&ui_painter->hotbar_labels[4], rect5, fill_color, style_data, text_data2);
}

static void setHotbarSurfaces(PlayerUIPainter* ui_painter) {
    /*
     * Ammo Surface
     */

    SDL_Surface* ammo_surface = IMG_Load("game_data/textures/player_ui/hotbar/Ammo.png");
    if (!ammo_surface) {
        SDL_Log("Failed to load image: %s", SDL_GetError());
    }
    // resize image
    SDL_Surface* resized_ammo_surface = SDL_ScaleSurface(ammo_surface, TEXTURE_RESOLUTION,
        TEXTURE_RESOLUTION, SDL_SCALEMODE_LINEAR);
    if (!resized_ammo_surface) {
        SDL_Log("Failed to scale surface: %s", SDL_GetError());
        return;
    }
    SDL_Surface* formatted_ammo_surface = SDL_ConvertSurface(resized_ammo_surface, SDL_PIXELFORMAT_RGBA8888);
    if (!formatted_ammo_surface) {
        SDL_Log("Failed to format surface: %s", SDL_GetError());
        return;
    }
    // storing and destroying
    ui_painter->ammo_surface = formatted_ammo_surface;
    SDL_DestroySurface(ammo_surface);
    SDL_DestroySurface(resized_ammo_surface);

    /*
     * Mag Surface
     */

    SDL_Surface* mag_surface = IMG_Load("game_data/textures/player_ui/hotbar/Magazine.png");
    if (!mag_surface) {
        SDL_Log("Failed to load image: %s", SDL_GetError());
    }
    // resize image
    SDL_Surface* resized_mag_surface = SDL_ScaleSurface(mag_surface, TEXTURE_RESOLUTION,
        TEXTURE_RESOLUTION, SDL_SCALEMODE_LINEAR);
    if (!resized_mag_surface) {
        SDL_Log("Failed to scale surface: %s", SDL_GetError());
        return;
    }
    SDL_Surface* formatted_mag_surface = SDL_ConvertSurface(resized_mag_surface, SDL_PIXELFORMAT_RGBA8888);
    if (!formatted_mag_surface) {
        SDL_Log("Failed to format surface: %s", SDL_GetError());
        return;
    }
    // storing and destroying
    ui_painter->mag_surface = formatted_mag_surface;
    SDL_DestroySurface(mag_surface);
    SDL_DestroySurface(resized_mag_surface);
}

static int setWeaponSurfaces(PlayerUIPainter* ui_painter) {
    // get directory
    const char* base_path = SDL_GetBasePath();
    if (!base_path) {
        SDL_Log("Could not determine base path: %s", SDL_GetError());
        return -1;
    }
    char target_dir[512];
    SDL_snprintf(target_dir, sizeof(target_dir), "%sgame_data/textures/player_ui/pov_weapons/", base_path);
    int num_files = 0;
    char** files = SDL_GlobDirectory(target_dir, NULL, SDL_GLOB_CASEINSENSITIVE, &num_files);
    if (files == NULL) {
        SDL_Log("Failed to list directory: %s", SDL_GetError());
        return -1;
    }
    // store images
    ui_painter->num_weapon_textures = 0;
    for (int i = 0; i < num_files; i++) {
        char surface_index_str[2];
        strncpy(surface_index_str, files[i], 2);
        char* end_ptr;
        const uint8_t surface_index = (int) strtol(surface_index_str, &end_ptr, 10);
        ui_painter->weapon_textures[surface_index] = malloc(sizeof(SDL_Texture));
        // load image
        char full_file_path[1024];
        SDL_snprintf(full_file_path, sizeof(full_file_path), "%s%s", target_dir, files[i]);
        SDL_Surface* surface = IMG_Load(full_file_path);
        if (!surface) {
            SDL_Log("Failed to load image %s: %s", files[i], SDL_GetError());
            SDL_free(files);
            return -1;
        }
        // resize image
        SDL_Surface* resized_surface = SDL_ScaleSurface(surface, TEXTURE_RESOLUTION, TEXTURE_RESOLUTION,
            SDL_SCALEMODE_LINEAR);
        if (!resized_surface) {
            SDL_Log("Failed to scale surface: %s", SDL_GetError());
            SDL_free(files);
            return -1;
        }
        SDL_Surface* formatted_surface = SDL_ConvertSurface(resized_surface, SDL_PIXELFORMAT_RGBA8888);
        if (!formatted_surface) {
            SDL_Log("Failed to format surface: %s", SDL_GetError());
            SDL_free(files);
            return -1;
        }
        // storing and destroying
        ui_painter->weapon_surfaces[surface_index] = formatted_surface;
        ui_painter->num_weapon_textures++;
        SDL_DestroySurface(surface);
        SDL_DestroySurface(resized_surface);
    }
    SDL_free(files);
    return num_files;
}