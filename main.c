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
#include "screen_logic/screen_logic.h"

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