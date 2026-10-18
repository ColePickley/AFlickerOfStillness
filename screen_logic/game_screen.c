#include "game_screen.h"

#include <math.h>
#include <stdlib.h>
#include <SDL3_image/SDL_image.h>

#include "screen_logic.h"
#include "../constants.h"
#include "../functions.h"
#include "../program_data.h"
#include "../ray_calculator.h"

void initGameScreen(GameScreen** game_screen) {
    *game_screen = malloc(sizeof(GameScreen));
    (*game_screen)->num_map_textures = setMapSurfaces(*game_screen);
    (*game_screen)->num_entity_textures = setEntitySurfaces(*game_screen);
}

void setGameScreenTextures(GameScreen* game_screen, SDL_Renderer* renderer) {
    for (int i = 0; i < game_screen->num_map_textures; i++) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, game_screen->map_surfaces[i]);
        if (!texture) {
            SDL_Log("Failed to create texture: %s", SDL_GetError());
            return;
        }
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        game_screen->map_textures[i] = texture;
    }
    for (int i = 0; i < game_screen->num_entity_textures; i++) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, game_screen->entity_surfaces[i]);
        if (!texture) {
            SDL_Log("Failed to create texture: %s", SDL_GetError());
            return;
        }
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        game_screen->entity_textures[i] = texture;
    }
}

void destroyGameScreen(GameScreen** game_screen) {
    if (*game_screen == NULL) return;
    for (int i = 0; i < (*game_screen)->num_map_textures; i++) {
        SDL_DestroyTexture((*game_screen)->map_textures[i]);
        SDL_DestroySurface((*game_screen)->map_surfaces[i]);
    }
    for (int i = 0; i < (*game_screen)->num_entity_textures; i++) {
        SDL_DestroyTexture((*game_screen)->entity_textures[i]);
        SDL_DestroySurface((*game_screen)->entity_surfaces[i]);
    }
    free(*game_screen);
    *game_screen = NULL;
}

/*
 * Painting
 */

int paintGameScreen(const GameScreen* game_screen, SDL_Renderer* renderer, GameData* game_data,
        const UserData* user_data) {
    setPlayerRays(game_data->player, game_data->num_map_rows, game_data->num_map_cols, game_data->map_grid);
    // int skyX = (int) ((double) WINDOW_WIDTH / 360.0 * (360.0 - player->rotation) * 3.0);
    // skyX -= skyX / WINDOW_WIDTH * WINDOW_WIDTH;
    // screen.blit(config.sky_textures[config.sky_texture_index], (skyX, 0))
    // screen.blit(config.sky_textures[config.sky_texture_index], (skyX - GAME_WIDTH, 0))
    for (int i = 0; i < NUM_RAYS; i++) {
        const double ray_length = game_data->player->rays[i].length;
        const double ray_degree = toRadians(game_data->player->rays[i].degree);
        const double adjusted_ray_length = max(0.0001, ray_length * cos(ray_degree));
        const float line_length = GRID_SIZE * WINDOW_HEIGHT / adjusted_ray_length;

        const int y = (WINDOW_HEIGHT - (int) line_length) / 2;
        const int x = i * LINE_WIDTH;
        const double angle = getRayAngle(game_data->player->rays[i].degree, game_data->player->rotation);
        const double pre_adjustment = (double) WINDOW_HEIGHT / 2.0 * (double) GRID_SIZE /
            cos(toRadians(game_data->player->rays[i].degree));
        int vert_res = LINE_WIDTH;
        if (user_data->graphics_quality == GRAPHICS_QUALITY_MEDIUM) {
            vert_res = LINE_WIDTH / 2;
        } else if (user_data->graphics_quality == GRAPHICS_QUALITY_HIGH) {
            vert_res = 1;
        }
        drawCeilingLine(game_screen, renderer, game_data, vert_res, i, y, angle, pre_adjustment);
        drawFloorLine(game_screen, renderer, game_data, vert_res, i, line_length, y, angle, pre_adjustment);
        drawWallLine(game_screen, renderer, game_data, angle, game_data->player->rays[i], line_length, x, y);
    }
    drawEntities(game_screen, renderer, game_data);
    return -1;
}

static void drawCeilingLine(const GameScreen* game_screen, SDL_Renderer* renderer, const GameData* game_data,
        const int vert_res, const int ray_num, const int start_y, const double angle, const double pre_adjustment) {
    for (int y = start_y; y >= 0; y -= vert_res) {
        const double dy = (double) WINDOW_HEIGHT / 2.0 - (double) y;
        const double post_adjustment = pre_adjustment / dy;
        int ceiling_x = getFloorX(game_data, angle, post_adjustment);
        const int texture_x = max(0, ceiling_x % GRID_SIZE);
        ceiling_x = max(0, min(game_data->num_map_cols - 1, ceiling_x / GRID_SIZE));
        int ceiling_y = getFloorY(game_data, angle, post_adjustment);
        const int texture_y = max(0, ceiling_y % GRID_SIZE);
        ceiling_y = max(0, min(game_data->num_map_rows - 1, ceiling_y / GRID_SIZE));
        const int texture_index = game_data->texture_locations[ceiling_y][ceiling_x][4];
        if (texture_index == 0) continue;
        SDL_Color color;
        setSurfacePixelColor(game_screen->map_surfaces[texture_index], texture_x, texture_y, &color);
        SDL_FRect rect = {.x = (float) (ray_num * LINE_WIDTH), .y = (float) y, .w = LINE_WIDTH, .h = (float) vert_res};
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &rect);
    }
}

static void drawFloorLine(const GameScreen* game_screen, SDL_Renderer* renderer, const GameData* game_data,
        const int vert_res, const int ray_num, const float line_length, const int start_y, const double angle,
        const double pre_adjustment) {
    for (int y = start_y + (int) line_length; y < WINDOW_HEIGHT; y += vert_res) {
        const double dy = y - WINDOW_HEIGHT / 2.0;
        const double post_adjustment = pre_adjustment / dy;
        int floor_x = getFloorX(game_data, angle, post_adjustment);
        const int texture_x = max(0, floor_x % GRID_SIZE);
        floor_x = max(0, min(game_data->num_map_cols - 1, floor_x / GRID_SIZE));
        int floor_y = getFloorY(game_data, angle, post_adjustment);
        const int texture_y = max(0, floor_y % GRID_SIZE);
        floor_y = max(0, min(game_data->num_map_rows - 1, floor_y / GRID_SIZE));
        const int texture_index = game_data->texture_locations[floor_y][floor_x][5];
        SDL_Color color;
        setSurfacePixelColor(game_screen->map_surfaces[texture_index], texture_x, texture_y, &color);
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_FRect rect = {.x = (float) (ray_num * LINE_WIDTH), .y = (float) y, .w = LINE_WIDTH, .h = (float) vert_res};
        SDL_RenderFillRect(renderer, &rect);
    }
}

static void drawWallLine(const GameScreen* game_screen, SDL_Renderer* renderer, const GameData* game_data,
        const double angle, const Ray ray, const float line_length, const int x, const int y) {
    // get texture
    const int side = getTextureSide(angle, ray);
    SDL_Texture* texture = getWallTexture(game_screen, game_data, angle, ray, side);
    float img_x;
    if (ray.vert_collision) {
        img_x = (float) round(GRID_SIZE - (int) ray.y % GRID_SIZE);
    } else {
        img_x = (float) round(GRID_SIZE - (int) ray.x % GRID_SIZE);
    }
    img_x = min(GRID_SIZE - 1, max(0, img_x));
    if (side <= 1) {
        img_x = GRID_SIZE - 1 - img_x;
    }
    const SDL_FRect src_rect = {.x = img_x, .y = 0, .w = 1, .h = TEXTURE_RESOLUTION};
    const SDL_FRect dst_rect = {.x = (float) x, .y = (float) y, .w = LINE_WIDTH, .h = line_length + LINE_WIDTH};
    SDL_RenderTexture(renderer, texture, &src_rect, &dst_rect);
    // transform and draw texture
    // if (line_length - WINDOW_HEIGHT > 0) {
        // img = pygame.transform.scale(img, (3, line_length))
        // rect = pygame.Rect(0, (line_length - GAME_HEIGHT) // 2, 3, GAME_HEIGHT)
        // img = img.subsurface(rect)
        // screen.blit(img, (x + BIRDS_EYE_WIDTH, 0))
    // } else {
        // img = pygame.transform.scale(img, (3, line_length))
        // screen.blit(img, (x + BIRDS_EYE_WIDTH, y))
    // }
}

static void drawEntities(const GameScreen* game_screen, SDL_Renderer* renderer, GameData* game_data) {
    setEntityVectors(game_data);
    quickSortEntities(game_data->entities, 0, game_data->num_entities - 1);
    const double player_vect_x = cos(toRadians(game_data->player->rotation - 90.0));
    const double player_vect_y = -1.0 * sin(toRadians(game_data->player->rotation - 90.0));
    for (int i = 0; i < game_data->num_entities; i++) {
        Entity* entity = game_data->entities[i];
        if (entity->vect_mag == 0) continue;
        const double angle = getEntityAngle(entity, player_vect_x, player_vect_y);
        const double adjusted_entity_vect_mag = entity->vect_mag * cos(toRadians(max(-30.0, min(30.0, angle))));

        const int entity_size = (int) round(entity->scale * WINDOW_HEIGHT / adjusted_entity_vect_mag);
        const int entity_height = (int) round(entity->height * 2 * WINDOW_HEIGHT / adjusted_entity_vect_mag);
        const int height_offset = (int) round((GRID_SIZE - entity->scale) * WINDOW_HEIGHT / adjusted_entity_vect_mag);
        const int entity_height_adjustment = entity_size - height_offset + entity_height;

        const int start_x = (int) ((angle + 30.0) * (double) LINE_WIDTH * 4.0 - (double) entity_size / 2.0);
        const int start_ray = start_x / LINE_WIDTH;
        const int end_ray = (start_x + entity_size) / LINE_WIDTH;
        for (int ray_num = max(0, start_ray); ray_num < min(NUM_RAYS, end_ray); ray_num++) {
            if (game_data->player->rays[ray_num].length < entity->vect_mag) continue;
            float texture_x = (float) round((ray_num - start_ray) / ((end_ray - start_ray) / 64.0));
            texture_x = min(63, max(0, texture_x));
            const int start_y = (int) ((WINDOW_HEIGHT - entity_height_adjustment) / 2.0);
            if (WINDOW_WIDTH / 2 - 10 <= ray_num * LINE_WIDTH && ray_num * LINE_WIDTH <= WINDOW_WIDTH / 2 + 10) {
                entity->in_scope = true;
            }
            SDL_Texture* texture = game_screen->entity_textures[entity->type_index];
            SDL_FRect src_rect = {
                .x = texture_x,
                .y = 0,
                .w = 1,
                .h = TEXTURE_RESOLUTION
            };
            SDL_FRect dst_rect = {
                .x = (float) (ray_num * LINE_WIDTH),
                .y = (float) start_y + LINE_WIDTH / 2.0f,
                .w = LINE_WIDTH,
                .h = (float) entity_size
            };
            SDL_RenderTexture(renderer, texture, &src_rect, &dst_rect);
        }
    }
}

/*
 * Event processing
 */

void gameScreenMousePressed(GameScreen* game_screen, SDL_FPoint mouse_pos) {

}

int gameScreenMouseReleased(GameScreen* game_screen, SDL_FPoint mouse_pos) {
    return -1;
}

void gameScreenMouseDragged(GameScreen* game_screen, const SDL_FPoint mouse_pos, const double prev_mouse_x,
        const GameData* game_data, const UserData* user_data) {
    rotatePlayer(game_data->player, user_data->sensitivity, mouse_pos.x - prev_mouse_x);
}

void gameScreenMouseMoved(GameScreen* game_screen, const SDL_FPoint mouse_pos, const double prev_mouse_x,
        const GameData* game_data, const UserData* user_data) {

}

void gameScreenKeyPressed(const SDL_Keycode keycode, const GameData* game_data) {
    switch (keycode) {
        case SDLK_W:
            game_data->player->w_pressed = true;
            break;
        case SDLK_A:
            game_data->player->a_pressed = true;
            break;
        case SDLK_S:
            game_data->player->s_pressed = true;
            break;
        case SDLK_D:
            game_data->player->d_pressed = true;
            break;
        default:
            break;
    }
}

void gameScreenKeyReleased(const SDL_Keycode keycode, const GameData* game_data) {
    switch (keycode) {
        case SDLK_W:
            game_data->player->w_pressed = false;
            break;
        case SDLK_A:
            game_data->player->a_pressed = false;
            break;
        case SDLK_S:
            game_data->player->s_pressed = false;
            break;
        case SDLK_D:
            game_data->player->d_pressed = false;
            break;
        default:
            break;
    }
}

int gameScreenKeyClicked(const SDL_Keycode keycode) {
    if (keycode == SDLK_ESCAPE) {
        return PAUSE_SCREEN;
    }
    return -1;
}

/*
 * Helper Functions
 */

static void setSurfacePixelColor(const SDL_Surface* surface, const int x, const int y, SDL_Color* out_color) {
    const Uint8* pixel_ptr = (const Uint8*)surface->pixels + y * surface->pitch + x * 4;
    const Uint32 pixel_value = *(const Uint32*)pixel_ptr;
    out_color->r = (pixel_value >> 24) & 0xFF;
    out_color->g = (pixel_value >> 16) & 0xFF;
    out_color->b = (pixel_value >> 8) & 0xFF;
    out_color->a = pixel_value & 0xFF;
}

static void setEntityVectors(const GameData* game_data) {
    for (int i = 0; i < game_data->num_entities; i++) {
        game_data->entities[i]->vect_x = game_data->entities[i]->x - game_data->player->x;
        game_data->entities[i]->vect_y = game_data->entities[i]->y - game_data->player->y;
        game_data->entities[i]->vect_mag = sqrt(pow(game_data->entities[i]->vect_x, 2) +
            pow(game_data->entities[i]->vect_y, 2));
    }
}

static void quickSortEntities(Entity* entities[MAX_NUM_ENTITIES], const int low, const int high) {
    if (low < high) {
        const double pivot = entities[high]->vect_mag;
        int i = low - 1;
        for (int j = low; j < high; j++) {
            if (entities[j]->vect_mag >= pivot) {
                i++;
                Entity* temp = entities[i];
                entities[i] = entities[j];
                entities[j] = temp;
            }
        }
        Entity* temp = entities[i + 1];
        entities[i + 1] = entities[high];
        entities[high] = temp;
        const int pi = i + 1;
        quickSortEntities(entities, low, pi - 1);
        quickSortEntities(entities, pi + 1, high);
    }
}

static double getEntityAngle(const Entity* entity, const double player_vect_x, const double player_vect_y) {
    const double dot_product = entity->vect_x * player_vect_x + entity->vect_y * player_vect_y;
    double angle = toDegrees(acos(dot_product / entity->vect_mag));
    const double determinant = entity->vect_x * player_vect_y - entity->vect_y * player_vect_x;
    if (determinant < 0)
        angle *= -1;
    return angle;
}

static int getTextureSide(const double angle, const Ray ray) {
    if (ray.vert_collision) {
        if (angle < 180) {
            return 3;
        }
        return 1;
    }
    if (angle < 90 || angle >= 270) {
        return 0;
    }
    return 2;
}

static SDL_Texture* getWallTexture(const GameScreen* game_screen, const GameData* game_data, const double angle,
        const Ray ray, const int side) {
    const int x = getWallTextureIndexX(game_data, angle, ray);
    const int y = getWallTextureIndexY(game_data, angle, ray);
    return game_screen->map_textures[game_data->texture_locations[y][x][side]];
}

static int getWallTextureIndexX(const GameData* game_data, const double angle, const Ray ray) {
    int ray_x = (int) ray.x;
    if (GRID_SIZE - fmod(ray.x, 64.0) < 0.00000001) {
        ray_x = (int) round(ray.x);
    }
    int x = ray_x / GRID_SIZE;
    if (angle >= 180 && ray.vert_collision) {
        x -= 1;
    }
    return max(0, min(game_data->num_map_cols - 1, x));
}

static int getWallTextureIndexY(const GameData* game_data, const double angle, const Ray ray) {
    int ray_y = (int) ray.y;
    if (GRID_SIZE - fmod(ray.y, 64.0) < 0.00000001) {
        ray_y = (int) round(ray.y);
    }
    int y = ray_y / GRID_SIZE;
    if (90.0 <= angle && angle < 270.0 && !ray.vert_collision) {
        y -= 1;
    }
    return max(0, min(game_data->num_map_rows - 1, y));
}

static int getFloorX(const GameData* game_data, const double angle, const double post_adjustment) {
    if (angle < 90) {
        return (int) (game_data->player->x + sin(toRadians(angle)) * post_adjustment);
    }
    if (angle < 180) {
        return (int) (game_data->player->x + cos(toRadians(angle - 90)) * post_adjustment);
    }
    if (angle < 270) {
        return (int) (game_data->player->x - sin(toRadians(angle - 180)) * post_adjustment);
    }
    return (int) (game_data->player->x - cos(toRadians(angle - 270)) * post_adjustment);
}

static int getFloorY(const GameData* game_data, const double angle, const double post_adjustment) {
    if (angle < 90) {
        return (int) (game_data->player->y + cos(toRadians(angle)) * post_adjustment);
    }
    if (angle < 180) {
        return (int) (game_data->player->y - sin(toRadians(angle - 90)) * post_adjustment);
    }
    if (angle < 270) {
        return (int) (game_data->player->y - cos(toRadians(angle - 180)) * post_adjustment);
    }
    return (int) (game_data->player->y + sin(toRadians(angle - 270)) * post_adjustment);
}

static double getRayAngle(const double ray_degree, const double player_rotation) {
    double angle = ray_degree + player_rotation;
    if (angle < 0.0) {
        angle += 360.0;
    } else if (angle >= 360.0) {
        angle -= 360.0;
    }
    return angle;
}

static int setMapSurfaces(GameScreen* game_screen) {
    // get directory
    const char* base_path = SDL_GetBasePath();
    if (!base_path) {
        SDL_Log("Could not determine base path: %s", SDL_GetError());
        return -1;
    }
    char target_dir[512];
    SDL_snprintf(target_dir, sizeof(target_dir), "%sgame_data/textures/surfaces/", base_path);
    int num_files = 0;
    char** files = SDL_GlobDirectory(target_dir, NULL, SDL_GLOB_CASEINSENSITIVE,
        &num_files);
    if (files == NULL) {
        SDL_Log("Failed to list directory: %s", SDL_GetError());
        return -1;
    }
    // slice image
    for (int i = 0; i < num_files; i++) {
        char surface_index_str[3];
        strncpy(surface_index_str, files[i], 2);
        char* end_ptr;
        const uint8_t surface_index = (int) strtol(surface_index_str, &end_ptr, 10);
        game_screen->map_textures[surface_index] = malloc(sizeof(SDL_Texture));
        // load image
        char full_file_path[1024];
        SDL_snprintf(full_file_path, sizeof(full_file_path), "%s%s", target_dir, files[i]);
        SDL_Surface* surface = IMG_Load(full_file_path);
        if (!surface) {
            SDL_Log("Failed to load image %s: %s", files[i], SDL_GetError());
            SDL_free(files);
            return -1;
        }
        // resize and format image
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
        game_screen->map_surfaces[surface_index] = formatted_surface;
        SDL_DestroySurface(surface);
        SDL_DestroySurface(resized_surface);
    }
    SDL_free(files);
    return num_files;
}

static int setEntitySurfaces(GameScreen* game_screen) {
    // get directory
    const char* base_path = SDL_GetBasePath();
    if (!base_path) {
        SDL_Log("Could not determine base path: %s", SDL_GetError());
        return -1;
    }
    char target_dir[512];
    SDL_snprintf(target_dir, sizeof(target_dir), "%sgame_data/textures/entities/", base_path);
    int num_files = 0;
    char** files = SDL_GlobDirectory(target_dir, NULL, SDL_GLOB_CASEINSENSITIVE, &num_files);
    if (files == NULL) {
        SDL_Log("Failed to list directory: %s", SDL_GetError());
        return -1;
    }
    // slice image
    for (int i = 0; i < num_files; i++) {
        char surface_index_str[3];
        strncpy(surface_index_str, files[i], 2);
        char* end_ptr;
        const uint8_t surface_index = (int) strtol(surface_index_str, &end_ptr, 10);
        game_screen->entity_textures[surface_index] = malloc(sizeof(SDL_Texture));
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
        game_screen->entity_surfaces[surface_index] = formatted_surface;
        SDL_DestroySurface(surface);
        SDL_DestroySurface(resized_surface);
    }
    SDL_free(files);
    return num_files;
}