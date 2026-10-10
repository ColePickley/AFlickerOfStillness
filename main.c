#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <SDL3/SDL.h>
#include "constants.h"
#include "program_data.h"
#include "screen_logic/painter.h"
#include "screen_logic/game_screen.h"
#include "screen_logic/start_screen.h"
#include "screen_logic/saves_screen.h"

static void handleEvents(SDL_Renderer* renderer, GameData* game_data, bool* program_running, bool* mouse_down,
    uint64_t* mouse_down_time, StartScreen* start_screen, const SavesScreen* saves_screen,
    const GameScreen* game_screen, uint8_t* current_screen, uint8_t* prev_screen, double* prev_mouse_x,
    UserData* user_data, SDL_Window* window);

static void mousePressed(SDL_FPoint pt, const StartScreen* start_screen,
    const SavesScreen* saves_screen, const GameScreen* game_screen, const uint8_t* current_screen);

static void mouseReleased(SDL_FPoint pt, StartScreen* start_screen, const SavesScreen* saves_screen,
    const GameScreen* game_screen, GameData* game_data, bool* program_running, uint8_t* current_screen,
    uint8_t* prev_screen, UserData* user_data, SDL_Window* window);

static void mouseDragged(SDL_FPoint pt, double* prev_mouse_x, const StartScreen* start_screen,
    const SavesScreen* saves_screen, const GameData* game_data, bool mouse_down, const uint8_t* current_screen,
    UserData* user_data);

static void printGameData(const GameData* game_data);

int main(const int argc, char* argv[]) {
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    srand((unsigned int)time(NULL));
    uint64_t mouse_down_time = 0;
    bool mouse_down = false;
    double prev_mouse_x = 0;
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

    if (!initPainterFont()) {
        SDL_Log("Critical Failure loading TrueType Font Engine!");
        SDL_Quit();
        return 1;
    }

    /*
     * Initialize screens
     */

    uint8_t current_screen = START_SCREEN;
    uint8_t prev_screen = START_SCREEN;
    StartScreen start_screen;
    initStartScreen(&start_screen, renderer, user_data);
    SavesScreen saves_screen;
    initSavesScreen(&saves_screen, renderer);
    GameScreen game_screen;
    initGameScreen(&game_screen, renderer);

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
        // clear screen
        SDL_SetRenderDrawColor(renderer, 38, 31, 28, 255);
        SDL_RenderClear(renderer);
        // paint screen
        switch (current_screen) {
            case START_SCREEN:
                paintStartScreen(&start_screen, renderer);
                break;
            case SAVES_SCREEN:
                paintSavesScreen(&saves_screen, renderer);
                break;
            case GAME_SCREEN:
                paintGameScreen(&game_screen, renderer, game_data, user_data);
                break;
            default:
                break;
        }
        // update player
        if (current_screen == GAME_SCREEN) {
            movePlayer(game_data->player, &game_data->map_grid);
        }
        // event handling
        handleEvents(renderer, game_data, &program_running, &mouse_down, &mouse_down_time, &start_screen, &saves_screen,
            &game_screen, &current_screen, &prev_screen, &prev_mouse_x, user_data, window);
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
    destroyStartScreen(&start_screen);
    destroySavesScreen(&saves_screen);
    destroyGameScreen(&game_screen);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    destroyPainterFont();
    SDL_Quit();
    printf("\nProgram run successfully!\n");
    return 0;
}

static void handleEvents(SDL_Renderer* renderer, GameData* game_data, bool* program_running, bool* mouse_down,
        uint64_t* mouse_down_time, StartScreen* start_screen, const SavesScreen* saves_screen,
        const GameScreen* game_screen, uint8_t* current_screen, uint8_t* prev_screen, double* prev_mouse_x, UserData* user_data,
        SDL_Window* window) {
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
            *mouse_down = true;
            if (event.button.button == SDL_BUTTON_LEFT && mouse_in_bounds) {
                mousePressed(pt, start_screen, saves_screen, game_screen, current_screen);
                *mouse_down_time = SDL_GetPerformanceCounter();
            }
        } else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            *mouse_down = false;
            if (event.button.button == SDL_BUTTON_LEFT && mouse_in_bounds) {
                mouseReleased(pt, start_screen, saves_screen, game_screen, game_data, program_running, current_screen,
                    prev_screen, user_data, window);
                const uint64_t mouseUpTime = SDL_GetPerformanceCounter();
                const double timeElapsed = (double) (mouseUpTime - *mouse_down_time) /
                    (double) SDL_GetPerformanceFrequency();
                if (timeElapsed < 0.2) {
                    // mouse clicked
                }
            }
        } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
            const SDL_FPoint motion_pt = { .x = event.motion.x, .y = event.motion.y };
            const bool motion_in_bounds = motion_pt.x >= 0 && motion_pt.y >= 0 &&
                motion_pt.x <= WINDOW_WIDTH && motion_pt.y <= WINDOW_HEIGHT;
            if (*mouse_down && motion_in_bounds) {
                mouseDragged(pt, prev_mouse_x, start_screen, saves_screen, game_data, *mouse_down, current_screen,
                    user_data);
            }
        } else if (event.type == SDL_EVENT_KEY_DOWN) {
            if (*current_screen == GAME_SCREEN) {
                if (event.key.key == SDLK_W) {
                    game_data->player->w_pressed = true;
                } else if (event.key.key == SDLK_A) {
                    game_data->player->a_pressed = true;
                } else if (event.key.key == SDLK_S) {
                    game_data->player->s_pressed = true;
                } else if (event.key.key == SDLK_D) {
                    game_data->player->d_pressed = true;
                }
            }
        } else if (event.type == SDL_EVENT_KEY_UP) {
            if (*current_screen == GAME_SCREEN) {
                if (event.key.key == SDLK_W) {
                    game_data->player->w_pressed = false;
                } else if (event.key.key == SDLK_A) {
                    game_data->player->a_pressed = false;
                } else if (event.key.key == SDLK_S) {
                    game_data->player->s_pressed = false;
                } else if (event.key.key == SDLK_D) {
                    game_data->player->d_pressed = false;
                }
            }
        }
    }
}

static void mousePressed(const SDL_FPoint pt, const StartScreen* start_screen, const SavesScreen* saves_screen,
        const GameScreen* game_screen, const uint8_t* current_screen) {
    switch (*current_screen) {
        case START_SCREEN:
            startScreenMousePressed(start_screen, pt);
            break;
        case SAVES_SCREEN:
            savesScreenMousePressed(saves_screen, pt);
            break;
        case GAME_SCREEN:
            // gameScreenMousePressed(saves_screen, pt);
            break;
        default:
            break;
    }
}

static void mouseReleased(const SDL_FPoint pt, StartScreen* start_screen, const SavesScreen* saves_screen,
        const GameScreen* game_screen, GameData* game_data, bool* program_running, uint8_t* current_screen,
        uint8_t* prev_screen, UserData* user_data, SDL_Window* window) {
    switch (*current_screen) {
        case START_SCREEN:
            startScreenMouseReleased(start_screen, pt, program_running, current_screen, prev_screen, user_data, window);
            break;
        case SAVES_SCREEN:
            savesScreenMouseReleased(saves_screen, pt, game_data, current_screen, prev_screen);
            break;
        case GAME_SCREEN:
            // gameScreenMouseReleased(saves_screen, pt);
            break;
        default:
            break;
    }
}

static void mouseDragged(const SDL_FPoint pt, double* prev_mouse_x, const StartScreen* start_screen,
        const SavesScreen* saves_screen, const GameData* game_data, const bool mouse_down,
        const uint8_t* current_screen, UserData* user_data) {
    switch (*current_screen) {
        case START_SCREEN:
            startScreenMouseDragged(start_screen, pt, user_data);
            break;
        case SAVES_SCREEN:
            savesScreenMouseDragged(saves_screen, pt);
            break;
        case GAME_SCREEN:
            if (mouse_down) {
                rotatePlayer(game_data->player, user_data->sensitivity, pt.x - *prev_mouse_x);
            }
            break;
        default:
            break;
    }
    *prev_mouse_x = pt.x;
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
        printf("\t\t\t Type Index: %d\n", game_data->entities[i]->type_index);
        printf("\t\t\t X: %lf\n", game_data->entities[i]->x);
        printf("\t\t\t Y: %lf\n", game_data->entities[i]->y);
    }
    printf("\t Sky Texture: %d\n", game_data->sky_texture_index);
    printf("\t Player:\n");
    printf("\t\t X: %lf\n", game_data->player->x);
    printf("\t\t Y: %lf\n", game_data->player->y);
    printf("\t\t Rotation: %lf\n", game_data->player->rotation);
}