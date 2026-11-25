#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <SDL3/SDL.h>

#include "astar.h"
#include "constants.h"
#include "event_handling.h"
#include "program_data.h"
#include "screen_logic/screen_logic.h"

static int updateGame(GameData* game_data, SDL_Window* window);

static void printGameData(const GameData* game_data);

int main() {
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    EventData event_data = {
        .mouse_down = false,
        .mouse_down_time = 0,
        .key_press_time = 0,
        .current_keycode = 0
    };
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

    if (!initPainter()) {
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
        paintScreen(screen_data->current_screen, false, screen_data, renderer, game_data, user_data, window);
        // update game
        int screen_change = -1;
        if (screen_data->current_screen == GAME_SCREEN) {
            screen_change = updateGame(game_data, window);
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
            printf("\rFPS: %.2f", 1.0 / time_elapsed);
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
    destroyPainter();
    SDL_Quit();
    printf("\nProgram run successfully!\n");
    return 0;
}

static int updateGame(GameData* game_data, SDL_Window* window) {
    updatePlayer(game_data->player, &game_data->map_grid);
    movePlayer(game_data->player, &game_data->map_grid);
    if (game_data->player->primary_selected && game_data->player->primary_weapon != NULL &&
            game_data->player->primary_weapon->type == WEAPON_AUTOMATIC && game_data->player->attacking) {
        playerRangedAttack(game_data->player, &game_data->entities, game_data->num_entities);
    }

    /*
     * Check if the player has hit a checkpoint
     */

    const SDL_Point player_pos = {
        .x = (int) game_data->player->x / GRID_SIZE,
        .y = (int) game_data->player->y / GRID_SIZE
    };
    if (game_data->map_grid[player_pos.y][player_pos.x] == CELL_CHECKPOINT &&
            !game_data->player->force_checkpoint_inactive) {
        bool new_checkpoint = true;
        if (game_data->current_checkpoint != NULL && game_data->current_checkpoint->x != player_pos.x &&
                game_data->current_checkpoint->y != player_pos.y) {
            for (int i = 0; i < game_data->num_hit_checkpoints; i++) {
                if (game_data->hit_checkpoints[i].x == player_pos.x &&
                        game_data->hit_checkpoints[i].y == player_pos.y) {
                    new_checkpoint = false;
                }
            }
        } else if (game_data->current_checkpoint != NULL) {
            game_data->player->checkpoint_active = true;
            new_checkpoint = false;
            if (!SDL_SetWindowRelativeMouseMode(window, false)) {
                SDL_Log("Could not enable relative mouse mode: %s", SDL_GetError());
            }
        }
        if (new_checkpoint) {
            game_data->player->checkpoint_active = true;
            if (game_data->last_checkpoint != NULL) {
                destroyCheckpoint(&game_data->last_checkpoint);
            }
            if (game_data->current_checkpoint != NULL) {
                game_data->last_checkpoint = game_data->current_checkpoint;
            }
            game_data->current_checkpoint = dequeueCheckpoint(game_data->checkpoint_queue);
            game_data->hit_checkpoints[game_data->num_hit_checkpoints] = (SDL_Point) {
                .x = game_data->current_checkpoint->x,
                .y = game_data->current_checkpoint->y
            };
            game_data->num_hit_checkpoints++;
            if (!SDL_SetWindowRelativeMouseMode(window, false)) {
                SDL_Log("Could not enable relative mouse mode: %s", SDL_GetError());
            }
        }
    } else if (game_data->map_grid[player_pos.y][player_pos.x] != CELL_CHECKPOINT) {
        game_data->player->checkpoint_active = false;
        game_data->player->force_checkpoint_inactive = false;
        if (!SDL_SetWindowRelativeMouseMode(window, true)) {
            SDL_Log("Could not enable relative mouse mode: %s", SDL_GetError());
        }
    }

    /*
     * Check player-entity collisions
     */

    int i = 0;
    while (i < game_data->num_entities) {
        if (!playerEntityCollisionCheck(game_data, i)) {
            i++;
        }
    }

    /*
     * Check entity-entity collisions and dead enemies
     */

    i = 0;
    while (i < game_data->num_entities) {
        bool entity_removed = false;
        if (game_data->entities[i]->subtype_id == ENTITY_SUBTYPE_PROJECTILE) {
            for (int j = 0; j < game_data->num_entities; j++) {
                if (i == j) continue;
                entity_removed = entityEntityCollisionCheck(game_data, i, j);
                if (entity_removed) break;
            }
        } else if (getEntityType(game_data->entities[i]) == ENTITY_TYPE_ENEMY && game_data->entities[i]->health <= 0) {
            removeEntity(game_data, i);
            entity_removed = true;
        }
        if (!entity_removed) {
            i++;
        }
    }

    /*
     * Check if player has died
     */

    int screen_change = -1;
    if (game_data->player->health <= 0) {
        screen_change = DEATH_SCREEN;
    }

    /*
     * move entities
     */

    i= 0;
    while (i < game_data->num_entities) {
        bool entity_removed = false;
        const int entity_type = getEntityType(game_data->entities[i]);
        if (entity_type == ENTITY_TYPE_ENEMY) {
            setEntityDirection(game_data->entities[i], game_data);
            moveEnemy(game_data->entities[i], &game_data->map_grid, game_data->entities, game_data->num_entities);
        } else if (entity_type == ENTITY_TYPE_PROJECTILE) {
            moveProjectile(game_data->entities[i], &game_data->map_grid, &entity_removed);
            if (entity_removed) {
                removeEntity(game_data, i);
            }
        }
        if (!entity_removed) {
            i++;
        }
    }

    return screen_change;
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
        const CheckpointPage* current_page = current_checkpoint->current_page;
        int page_num = 0;
        while (current_page != NULL) {
            printf("\t\t\t Page: %d\n", page_num);
            for (int i = 0; current_page->page_labels[i] != NULL; i++) {
                for (int j = 0; j < current_page->page_labels[i]->text_data.num_lines; j++) {
                    printf("\t\t\t\t %s\n", current_page->page_labels[i]->text_data.text[j]);
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