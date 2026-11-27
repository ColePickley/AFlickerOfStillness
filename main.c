#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <SDL3/SDL.h>

#include "astar.h"
#include "constants.h"
#include "event_handling.h"
#include "game_updates.h"
#include "program_data.h"
#include "file_handling/save_handling.h"
#include "screen_logic/screen_logic.h"

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
        ScreenChange screen_change = NO_CHANGE;
        if (screen_data->current_screen == GAME_SCREEN) {
            screen_change = updateGame(game_data, window);
        }
        if (screen_change != NO_CHANGE) {
            changeScreen(screen_data, screen_change, renderer, game_data, user_data, window);
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