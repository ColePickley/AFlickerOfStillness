#include "player_ui_painter.h"

#include <stdio.h>

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

    /*
     * POV Weapon
     */

    const SDL_FRect pov_weapon_rect = {
        .x = WINDOW_WIDTH - 700.0f,
        .y = WINDOW_HEIGHT - HOTBAR_HEIGHT - 400.0f,
        .w = 700.0f,
        .h = 400.0f
    };
    if (player->primary_selected && player->primary_weapon != NULL) {
        SDL_RenderTexture(renderer, ui_painter->weapon_textures[player->primary_weapon->weapon_id], NULL,
            &pov_weapon_rect);
    } else if (!player->primary_selected && player->secondary_weapon != NULL) {
        SDL_RenderTexture(renderer, ui_painter->weapon_textures[player->secondary_weapon->weapon_id], NULL,
            &pov_weapon_rect);
    }

    /*
     * Hotbar Background
     */

    const SDL_FRect hotbar_rect = {
        .x = 0,
        .y = WINDOW_HEIGHT - HOTBAR_HEIGHT,
        .w = WINDOW_WIDTH,
        .h = HOTBAR_HEIGHT
    };
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    SDL_RenderFillRect(renderer, &hotbar_rect);
    // Ammo and Mag label text
    if (player->primary_weapon != NULL) {
        char ammo_label_text[6];
        snprintf(ammo_label_text, sizeof (ammo_label_text), "%2d%s%2d", player->primary_weapon->ammo_count,
            "/", player->primary_weapon->max_ammo_count);
        char mag_label_text[6];
        snprintf(mag_label_text, sizeof (mag_label_text), "%2d%s%2d", player->primary_weapon->mag_count,
            "/", player->primary_weapon->max_mag_count);
        for (int i = 0; i < 5; i++) {
            ui_painter->hotbar_labels[0]->text_data.text[0][i] = ammo_label_text[i];
            ui_painter->hotbar_labels[1]->text_data.text[0][i] = mag_label_text[i];
        }
        ui_painter->hotbar_labels[0]->text_data.text[0][5] = '\0';
        ui_painter->hotbar_labels[1]->text_data.text[0][5] = '\0';
    }
    // weapon label background colors
    if (player->primary_selected) {
        ui_painter->hotbar_labels[2]->fill_color = (SDL_Color) {.r = 200, .g = 170, .b = 170, .a = 255};
        ui_painter->hotbar_labels[3]->fill_color = (SDL_Color) {.r = 200, .g = 200, .b = 200, .a = 255};
    } else {
        ui_painter->hotbar_labels[2]->fill_color = (SDL_Color) {.r = 200, .g = 200, .b = 200, .a = 255};
        ui_painter->hotbar_labels[3]->fill_color = (SDL_Color) {.r = 200, .g = 170, .b = 170, .a = 255};
    }
    // hotbar labels
    for (int i = 0; i < 5; i++) {
        paintLabel(renderer, ui_painter->hotbar_labels[i]);
    }

    /*
     * Ammo and Mag Textures
     */

    const SDL_FRect ammo_rect = {
        .x = 27.5f,
        .y = WINDOW_HEIGHT - HOTBAR_HEIGHT + 27.5f,
        .w = 120.0f,
        .h = 120.0f,
    };
    SDL_RenderTexture(renderer, ui_painter->ammo_texture, NULL, &ammo_rect);

    const SDL_FRect mag_rect = {
        .x = 452.5f,
        .y = WINDOW_HEIGHT - HOTBAR_HEIGHT + 27.5f,
        .w = 120.0f,
        .h = 120.0f,
    };
    SDL_RenderTexture(renderer, ui_painter->mag_texture, NULL, &mag_rect);

    /*
     * Weapon textures
     */

    if (player->primary_weapon != NULL) {
        const SDL_FRect weapon_rect = {
            .x = 887.5f,
            .y = WINDOW_HEIGHT - HOTBAR_HEIGHT + 15.0f,
            .w = 145.0f,
            .h = HOTBAR_HEIGHT - 30,
        };
        SDL_RenderTexture(renderer, entity_textures[player->primary_weapon->entity_id], NULL,
            &weapon_rect);
    }
    if (player->secondary_weapon != NULL) {
        const SDL_FRect weapon_rect = {
            .x = 1142.5f,
            .y = WINDOW_HEIGHT - HOTBAR_HEIGHT + 15.0f,
            .w = 145.0f,
            .h = HOTBAR_HEIGHT - 30,
        };
        SDL_RenderTexture(renderer, entity_textures[player->secondary_weapon->entity_id], NULL,
            &weapon_rect);
    }

    /*
     * Health bar
     */

    const SDL_FRect health_bar_rect = {
        .x = 1365.0f,
        .y = WINDOW_HEIGHT - HOTBAR_HEIGHT + 25.0f,
        .w = WINDOW_WIDTH - 1390.0f,
        .h = HOTBAR_HEIGHT - 50.0f,
    };
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_RenderFillRect(renderer, &health_bar_rect);

    const int health_bar_y = (int) health_bar_rect.y + 5;
    const int health_bar_width = (int) health_bar_rect.w - 10;
    const int health_bar_height = (int) health_bar_rect.h - 10;
    const int health_bar_start_y =
        health_bar_y + (int) ((1.0f - (float) player->health / 100.0f) * (float) health_bar_height);
    for (int i = health_bar_start_y; i < health_bar_y + health_bar_height; i++) {
        const uint8_t r = (uint8_t) ((float) (i - health_bar_y) / (float) health_bar_height * 255.0f);
        const uint8_t g = (uint8_t) ((1.0f - (float) (i - health_bar_y) / (float) health_bar_height) * 255.0f);
        SDL_SetRenderDrawColor(renderer, min(255, r), min(255, g), 0, 255);
        SDL_RenderLine(renderer, 1370.0f, (float) i, 1370.0f + (float) health_bar_width, (float) i);
    }
}

static void paintKeys(PlayerUIPainter* ui_painter, SDL_Renderer* renderer, const Player* player,
        SDL_Texture* const entity_textures[MAX_NUM_MAP_TEXTURES]) {
    for (int i = 0; i < player->num_keys; i++) {
        SDL_FRect dst_rect = {
            .x = WINDOW_WIDTH - 100.0f,
            .y = (float) i * 100.0f,
            .w = 100.0f,
            .h = 100.0f,
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