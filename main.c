#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <SDL3/SDL.h>

#include "astar.h"
#include "constants.h"
#include "functions.h"
#include "program_data.h"
#include "screen_logic/screen_logic.h"

typedef struct {
    bool mouse_down;
    uint64_t mouse_down_time;
    uint64_t key_press_time;
    double prev_mouse_x;
    SDL_Keycode current_keycode;
}EventData;

static void handleEvents(ScreenData* screen_data, SDL_Renderer* renderer, GameData** game_data, bool* program_running,
    EventData* event_data, UserData* user_data, SDL_Window* window);

static int updateGame(GameData* game_data);

static void collectAmmo(GameData* game_data, int entity_index);

static void collectHealth(GameData* game_data, int entity_index);

static void collectWeapon(GameData* game_data, int entity_index);

static void collectKey(GameData* game_data, int entity_index);

static void printGameData(const GameData* game_data);

int main() {
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    srand((unsigned int) time(NULL));
    EventData event_data =
        {.mouse_down = false, .mouse_down_time = 0, .key_press_time = 0, .prev_mouse_x = 0, .current_keycode = 0};
    UserData* user_data;
    initUserData(&user_data);
    loadUserData(user_data);

    /*
     * Initialize SDL
     */
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL could not initialize! Error: %s", SDL_GetError());
        return 1;
    }
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    if (!SDL_CreateWindowAndRenderer("A Flicker of Stillness: The Video Game",
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Window/Renderer creation failed! Error: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderLogicalPresentation(renderer, WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_LOGICAL_PRESENTATION_LETTERBOX);
    SDL_SetWindowFullscreen(window, user_data->fullscreen);

    /*
     * Initialize painter
     */

    if (!initPainterFonts()) {
        SDL_Log("Critical Failure loading TrueType Font Engine!");
        SDL_Quit();
        return 1;
    }

    /*
     * Initialize screen data
     */

    ScreenData* screen_data;
    initScreenData(&screen_data, renderer, user_data);

    /*
     * Initialize Game Data
     */

    GameData* game_data;
    initGameData(&game_data);

    /*
     *  main game loop
     */

    bool program_running = true;
    int frame_count = 0;
    while (program_running) {
        const uint64_t start_time = SDL_GetPerformanceCounter();
        // paint screen
        paintScreen(screen_data->current_screen, screen_data, renderer, game_data, user_data);
        // update game
        int screen_change = -1;
        if (screen_data->current_screen == GAME_SCREEN) {
            screen_change = updateGame(game_data);
        }
        if (screen_change != -1) {
            changeScreen(screen_data, screen_change, renderer, game_data, user_data);
        }
        // event handling
        handleEvents(screen_data, renderer, &game_data, &program_running, &event_data, user_data, window);
        // render graphics
        SDL_RenderPresent(renderer);
        // frame rate control
        uint64_t end_time = SDL_GetPerformanceCounter();
        double time_elapsed = (double) (end_time - start_time) / (double) SDL_GetPerformanceFrequency();
        const double target_frame_rate = 1.0 / user_data->frame_rate_cap;
        if (time_elapsed < target_frame_rate) {
            const uint32_t delay_ms = (uint32_t)((target_frame_rate - time_elapsed) * 1000.0);
            SDL_Delay(delay_ms);
        }
        frame_count++;
        if (frame_count >= 15) {
            end_time = SDL_GetPerformanceCounter();
            time_elapsed = (double) (end_time - start_time) / (double) SDL_GetPerformanceFrequency();
            // printf("\rFPS: %.2f | ", 1.0 / time_elapsed);
            // printf("Start Screen: %s | ", screen_data->start_screen != NULL ? "Loaded" : "Unloaded");
            // printf("Saves Screen: %s | ", screen_data->saves_screen != NULL ? "Loaded" : "Unloaded");
            // printf("Game Screen: %s | ", screen_data->game_screen != NULL ? "Loaded" : "Unloaded");
            // printf("Pause Screen: %s | ", screen_data->pause_screen != NULL ? "Loaded" : "Unloaded");
            // printf("Death Screen: %s | ", screen_data->death_screen != NULL ? "Loaded" : "Unloaded");
            // printf("Settings Screen: %s | ", screen_data->settings_screen != NULL ? "Loaded" : "Unloaded");
            fflush(stdout);
            frame_count = 0;
        }
    }
    // end
    destroyScreenData(&screen_data);
    destroyGameData(&game_data);
    destroyUserData(&user_data);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    destroyPainterFonts();
    SDL_Quit();
    printf("\nProgram run successfully!\n");
    return 0;
}

static void handleEvents(ScreenData* screen_data, SDL_Renderer* renderer, GameData** game_data, bool* program_running,
        EventData* event_data, UserData* user_data, SDL_Window* window) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            *program_running = false;
        }

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP ||
                event.type == SDL_EVENT_MOUSE_MOTION) {
            SDL_ConvertEventToRenderCoordinates(renderer, &event);
        }
        const SDL_FPoint pt = { .x = event.button.x, .y = event.button.y };
        const bool mouse_in_bounds = pt.x >= 0 && pt.y >= 0 && pt.x <= WINDOW_WIDTH && pt.y <= WINDOW_HEIGHT;

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            event_data->mouse_down = true;
            if (event.button.button == SDL_BUTTON_LEFT && mouse_in_bounds) {
                mousePressed(screen_data, pt);
                event_data->mouse_down_time = SDL_GetPerformanceCounter();
            }
        } else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            event_data->mouse_down = false;
            if (event.button.button == SDL_BUTTON_LEFT && mouse_in_bounds) {
                mouseReleased(screen_data, pt, renderer, program_running, game_data, user_data, window);
                const uint64_t mouse_up_time = SDL_GetPerformanceCounter();
                const double time_elapsed = (double) (mouse_up_time - event_data->mouse_down_time) /
                    (double) SDL_GetPerformanceFrequency();
                if (time_elapsed < 0.2) {
                    mouseClicked(screen_data, pt);
                }
            }
        } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
            const SDL_FPoint motion_pt = { .x = event.motion.x, .y = event.motion.y };
            const bool motion_in_bounds = motion_pt.x >= 0 && motion_pt.y >= 0 &&
                motion_pt.x <= WINDOW_WIDTH && motion_pt.y <= WINDOW_HEIGHT;
            // mouseMoved(screen_data, pt, event_data->prev_mouse_x, *game_data, user_data);
            if (event_data->mouse_down && motion_in_bounds) {
                mouseDragged(screen_data, pt, event_data->prev_mouse_x, *game_data, user_data);
                event_data->prev_mouse_x = pt.x;
            }
        } else if (event.type == SDL_EVENT_KEY_DOWN) {
            event_data->current_keycode = event.key.key;
            event_data->key_press_time = SDL_GetPerformanceCounter();
            keyPressed(screen_data, event.key.key, *game_data);
        } else if (event.type == SDL_EVENT_KEY_UP) {
            keyReleased(screen_data, event.key.key, *game_data);
            if (event.key.key == event_data->current_keycode) {
                const uint64_t key_up_time = SDL_GetPerformanceCounter();
                const double time_elapsed = (double) (key_up_time - event_data->mouse_down_time) /
                    (double) SDL_GetPerformanceFrequency();
                if (time_elapsed < 100) {
                    keyClicked(screen_data, event_data->current_keycode, renderer, *game_data, user_data);
                }
            }
        }
    }
}

static int updateGame(GameData* game_data) {
    movePlayer(game_data->player, &game_data->map_grid);

    /*
     * Check entity collisions
     */

    for (int i = 0; i < game_data->num_entities; i++) {
        if (fabs(game_data->entities[i]->x - game_data->player->x) < 10 &&
                fabs(game_data->entities[i]->y - game_data->player->y) < 10) {
            switch (game_data->entities[i]->subtype_id) {
                case ENTITY_SUBTYPE_AESTHETIC:
                case ENTITY_SUBTYPE_ENEMY_PROJECTILE:
                    break;
                case ENTITY_SUBTYPE_COLLECTABLE_AMMO:
                    collectAmmo(game_data, i);
                    break;
                case ENTITY_SUBTYPE_COLLECTABLE_HEALTH:
                    collectHealth(game_data, i);
                    break;
                case ENTITY_SUBTYPE_COLLECTABLE_WEAPON:
                    collectWeapon(game_data, i);
                    break;
                case ENTITY_SUBTYPE_COLLECTABLE_KEY:
                    collectKey(game_data, i);
                    break;
                case ENTITY_SUBTYPE_PROJECTILE:
                    game_data->player->health -= game_data->entities[i]->projectile_damage;
                    removeEntity(game_data, i);
                    break;
                default:
                    break;
            }
        }
    }
    int screen_change = -1;
    if (game_data->player->health <= 0) {
        screen_change = DEATH_SCREEN;
    }
    for (int i = 0; i < game_data->num_entities; i++) {
        if (getEntityType(game_data->entities[i]) == ENTITY_TYPE_ENEMY) {
            setEntityDirection(game_data->entities[i], game_data);
            moveEntity(game_data->entities[i], &game_data->map_grid, game_data->entities, game_data->num_entities);
        }
    }
    return screen_change;
}

static void collectAmmo(GameData* game_data, const int entity_index) {
    if (game_data->player->primary_weapon == NULL) return;
    if (game_data->player->primary_weapon->mag_count == game_data->player->primary_weapon->max_mag_count) return;
    if (game_data->entities[entity_index]->ammo_type == game_data->player->primary_weapon->ammo_type) {
        game_data->player->primary_weapon->mag_count += game_data->entities[entity_index]->mag_amount;
        game_data->player->primary_weapon->mag_count =
            min(game_data->player->primary_weapon->max_mag_count,
            game_data->player->primary_weapon->mag_count);
        removeEntity(game_data, entity_index);
    }
}

static void collectWeapon(GameData* game_data, const int entity_index) {
    Weapon* new_weapon;
    initWeapon(&new_weapon, game_data->entities[entity_index]->weapon_index,
        game_data->entities[entity_index]->texture_id, game_data->entities[entity_index]->mag_count,
        game_data->entities[entity_index]->ammo_count);
    Weapon* old_weapon;
    if (isPrimaryWeapon(new_weapon)) {
        old_weapon = game_data->player->primary_weapon;
        game_data->player->primary_weapon = new_weapon;
    } else {
        old_weapon = game_data->player->secondary_weapon;
        game_data->player->secondary_weapon = new_weapon;
    }
    if (old_weapon != NULL) {
        Entity* new_entity;
        initDefaultEntity(&new_entity, old_weapon->entity_id, game_data->entities[entity_index]->x,
            game_data->entities[entity_index]->y);
        new_entity->mag_count = old_weapon->mag_count;
        new_entity->ammo_count = old_weapon->ammo_count;
        addEntity(game_data, new_entity);
    }
    removeEntity(game_data, entity_index);
}

static void collectHealth(GameData* game_data, const int entity_index) {
    if (game_data->player->health < MAX_PLAYER_HEALTH) {
        game_data->player->health += game_data->entities[entity_index]->health_amount;
        game_data->player->health = min(game_data->player->health, MAX_PLAYER_HEALTH);
        removeEntity(game_data, entity_index);
    }
}

static void collectKey(GameData* game_data, const int entity_index) {
    if (game_data->player->num_keys < MAX_NUM_KEYS) {
        const int entity_id = game_data->entities[entity_index]->texture_id;
        const int grid_y = (int) (game_data->entities[entity_index]->y / 64);
        const int grid_x = (int) (game_data->entities[entity_index]->x / 64);
        const int chunk = game_data->chunk_grid[grid_y][grid_x];
        Key* new_key;
        initKey(&new_key, entity_id, chunk);
        game_data->player->keys[game_data->player->num_keys] = new_key;
        game_data->player->num_keys++;
        removeEntity(game_data, entity_index);
    }
}

static void printGameData(const GameData* game_data) {
    printf("Game Data:\n");
    printf("\t Map Grid:\n");
    for (int i = 0; i < game_data->num_map_rows; i++) {
        for (int j = 0; j < game_data->num_map_cols; j++) {
            printf("\t\t %d, ", game_data->map_grid[i][j]);
        }
        printf("\n");
    }
    printf("\t Chunk Grid:\n");
    for (int i = 0; i < game_data->num_map_rows; i++) {
        for (int j = 0; j < game_data->num_map_cols; j++) {
            printf("\t\t %d, ", game_data->chunk_grid[i][j]);
        }
        printf("\n");
    }
    printf("\t Texture Locations:\n");
    for (int x = 0; x < game_data->num_map_cols; x++) {
        for (int side = 0; side < 6; side++) {
            printf("\t\t ");
            for (int y = 0; y < game_data->num_map_rows; y++) {
                printf("%d, %d, %d, %d | ", y, x, side, game_data->texture_locations[y][x][side]);
            }
            printf("\n");
        }
    }
    printf("\t Checkpoints:\n");
    CheckpointQueue* temp_queue;
    initCheckpointQueue(&temp_queue);
    while (!isEmptyCheckpointQueue(game_data->checkpoint_queue)) {
        Checkpoint* current_checkpoint = dequeueCheckpoint(game_data->checkpoint_queue);
        enqueueCheckpoint(temp_queue, current_checkpoint);
        printf("\t\t x: %d, y: %d\n", current_checkpoint->x, current_checkpoint->y);
        const CheckpointPage* current_page = current_checkpoint->start_page;
        int page_num = 0;
        while (current_page != NULL) {
            printf("\t\t\t Page: %d\n", page_num);
            for (int i = 0; current_page->page_dialogue[i] != NULL; i++) {
                for (int j = 0; j < current_page->page_dialogue[i]->label->text_data.num_lines; j++) {
                    printf("\t\t\t\t %s\n", current_page->page_dialogue[i]->label->text_data.text[j]);
                }
                printf("\n");
            }
            current_page = current_page->next_page;
            page_num++;
        }
    }
    while (!isEmptyCheckpointQueue(temp_queue)) {
        enqueueCheckpoint(game_data->checkpoint_queue, dequeueCheckpoint(temp_queue));
    }
    printf("\t Entities:\n");
    for (int i = 0; i < game_data->num_entities; i++) {
        printf("\t\t Entity %d:\n", i);
        printf("\t\t\t Type Index: %d\n", game_data->entities[i]->subtype_id);
        printf("\t\t\t X: %lf\n", game_data->entities[i]->x);
        printf("\t\t\t Y: %lf\n", game_data->entities[i]->y);
    }
    printf("\t Sky Texture: %d\n", game_data->sky_texture_index);
    printf("\t Player:\n");
    printf("\t\t X: %lf\n", game_data->player->x);
    printf("\t\t Y: %lf\n", game_data->player->y);
    printf("\t\t Rotation: %lf\n", game_data->player->rotation);
}