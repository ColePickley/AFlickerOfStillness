#include "screen_data.h"

void initScreenData(ScreenData** screen_data, SDL_Renderer* renderer, const UserData* user_data) {
    *screen_data = malloc(sizeof(ScreenData));
    (*screen_data)->current_screen = START_SCREEN;
    (*screen_data)->prev_screen = START_SCREEN;
    initStartScreen(&(*screen_data)->start_screen);
    setStartScreenTextures((*screen_data)->start_screen, renderer);
    initSavesScreen(&(*screen_data)->saves_screen);
    setSavesScreenTextures((*screen_data)->saves_screen, renderer);
    initSettingsScreen(&(*screen_data)->settings_screen, user_data, (*screen_data)->prev_screen);
    setSettingsScreenTextures((*screen_data)->settings_screen, renderer);
    (*screen_data)->pause_screen = NULL;
    (*screen_data)->game_screen = NULL;

    (*screen_data)->is_loading_in_background = false;
    (*screen_data)->active_async_task = NULL;
    (*screen_data)->bg_thread = NULL;
}

void destroyScreenData(ScreenData** screen_data) {
    if (*screen_data == NULL) return;

    // Safely block and await background completion if closing down during an active screen transition
    if ((*screen_data)->is_loading_in_background && (*screen_data)->bg_thread) {
        SDL_WaitThread((*screen_data)->bg_thread, NULL);
        if ((*screen_data)->active_async_task) {
            free((*screen_data)->active_async_task);
        }
    }

    destroyStartScreen(&(*screen_data)->start_screen);
    destroySavesScreen(&(*screen_data)->saves_screen);
    destroyGameScreen(&(*screen_data)->game_screen);
    destroyPauseScreen(&(*screen_data)->pause_screen);
    destroySettingsScreen(&(*screen_data)->settings_screen);
    free(*screen_data);
    *screen_data = NULL;
}

static int asyncScreenChange(void* data) {
    ThreadLoadArgs* args = data;
    if (args->screen_data->prev_screen == START_SCREEN) {
        if (args->screen_id == SAVES_SCREEN) {
            initGameScreen(&args->screen_data->game_screen);
        }
    } else if (args->screen_data->prev_screen == SAVES_SCREEN) {
        if (args->screen_id == START_SCREEN) {
            initSettingsScreen(&args->screen_data->settings_screen, args->user_data, args->screen_data->current_screen);
        } else if (args->screen_id == GAME_SCREEN) {
            initPauseScreen(&args->screen_data->pause_screen);
        }
    } else if (args->screen_data->prev_screen == SETTINGS_SCREEN) {
        if (args->screen_id == START_SCREEN) {
            initSavesScreen(&args->screen_data->saves_screen);
        } else if (args->screen_id == PAUSE_SCREEN) {
            initGameScreen(&args->screen_data->game_screen);
            initStartScreen(&args->screen_data->start_screen);
        }
    } else if (args->screen_data->prev_screen == GAME_SCREEN) {
        if (args->screen_id == PAUSE_SCREEN) {
            initSettingsScreen(&args->screen_data->settings_screen, args->user_data, args->screen_data->current_screen);
            initStartScreen(&args->screen_data->start_screen);
        }
    } else if (args->screen_data->prev_screen == PAUSE_SCREEN) {
        if (args->screen_id == START_SCREEN) {
            initSavesScreen(&args->screen_data->saves_screen);
        }
    }
    args->is_done = true;
    return 0;
}

void changeScreen(ScreenData* screen_data, const int screen_id, SDL_Renderer* renderer, const UserData* user_data) {
    if (screen_id == -1) return;
    if (screen_data->is_loading_in_background) return;

    screen_data->prev_screen = screen_data->current_screen;
    screen_data->current_screen = screen_id;

    screen_data->is_loading_in_background = true;
    ThreadLoadArgs* args = malloc(sizeof(ThreadLoadArgs));
    args->screen_data = screen_data;
    args->user_data = user_data;
    args->renderer = renderer;
    args->screen_id = screen_id;
    args->is_done = false;
    screen_data->active_async_task = args;
    screen_data->bg_thread = SDL_CreateThread(asyncScreenChange, "GameLoadingWorker", args);
    if (!screen_data->bg_thread) {
        SDL_Log("Thread creation failed! Falling back to synchronous safety fallback.");
    }
}

void loadScreenTextures(ScreenData* screen_data, SDL_Renderer* renderer, const int screen_id) {
    if (screen_data->prev_screen == START_SCREEN) {
        if (screen_id == SAVES_SCREEN) {
            destroySettingsScreen(&screen_data->settings_screen);
            setGameScreenTextures(screen_data->game_screen, renderer);
        } else if (screen_id == SETTINGS_SCREEN) {
            destroySavesScreen(&screen_data->saves_screen);
        }
    } else if (screen_data->prev_screen == SAVES_SCREEN) {
        if (screen_id == START_SCREEN) {
            destroyGameScreen(&screen_data->game_screen);
            setSettingsScreenTextures(screen_data->settings_screen, renderer);
        } else if (screen_id == GAME_SCREEN) {
            destroyStartScreen(&screen_data->start_screen);
            destroySavesScreen(&screen_data->saves_screen);
            setPauseScreenTextures(screen_data->pause_screen, renderer);
        }
    } else if (screen_data->prev_screen == SETTINGS_SCREEN) {
        if (screen_id == START_SCREEN) {
            setSavesScreenTextures(screen_data->saves_screen, renderer);
        } else if (screen_id == PAUSE_SCREEN) {
            setGameScreenTextures(screen_data->game_screen, renderer);
            setStartScreenTextures(screen_data->start_screen, renderer);
        }
    } else if (screen_data->prev_screen == GAME_SCREEN) {
        if (screen_id == PAUSE_SCREEN) {
            setSettingsScreenTextures(screen_data->settings_screen, renderer);
            setStartScreenTextures(screen_data->start_screen, renderer);
        }
    } else if (screen_data->prev_screen == PAUSE_SCREEN) {
        if (screen_id == GAME_SCREEN) {
            destroySettingsScreen(&screen_data->settings_screen);
            destroyStartScreen(&screen_data->start_screen);
        } else if (screen_id == SETTINGS_SCREEN) {
            destroyGameScreen(&screen_data->game_screen);
            destroyStartScreen(&screen_data->start_screen);
        } else if (screen_id == START_SCREEN) {
            destroyPauseScreen(&screen_data->pause_screen);
            destroyGameScreen(&screen_data->game_screen);
            setSavesScreenTextures(screen_data->saves_screen, renderer);
            screen_data->settings_screen->prev_screen = START_SCREEN;
        }
    }
}

void paintScreen(const int screen_id, ScreenData* screen_data, SDL_Renderer* renderer, GameData* game_data,
        const UserData* user_data) {

    // --- ASYNC RUNTIME COMPLETION POLL ---
    if (screen_data->is_loading_in_background && screen_data->active_async_task) {
        if (screen_data->active_async_task->is_done) {
            // Clean up our finished worker thread resources completely
            SDL_WaitThread(screen_data->bg_thread, NULL);
            free(screen_data->active_async_task);
            screen_data->active_async_task = NULL;
            screen_data->bg_thread = NULL;
            screen_data->is_loading_in_background = false;
            loadScreenTextures(screen_data, renderer, screen_id);
        } else if (screen_data->prev_screen == PAUSE_SCREEN) {
            // --- DRAW YOUR LOADING SCREEN HERE ---
            // The background thread is busy! Render something animated to keep UI responsive.

            // screen_id = screen_data->prev_screen;

            // Call a helper function here to display text like "Loading Assets..."
            // or step up a rotating layout spinner frame-by-frame.
            return; // Interrupt processing so it skips trying to render the uninitialized game screen!
        }
    }

    // clear screen
    SDL_SetRenderDrawColor(renderer, 38, 31, 28, 255);
    SDL_RenderClear(renderer);

    int screen_change = -1;
    switch (screen_id) {
        case START_SCREEN:
            if (screen_data->start_screen == NULL) return;
            screen_change = paintStartScreen(screen_data->start_screen, renderer);
            break;
        case SAVES_SCREEN:
            if (screen_data->saves_screen == NULL) return;
            screen_change = paintSavesScreen(screen_data->saves_screen, renderer);
            break;
        case GAME_SCREEN:
            if (screen_data->game_screen == NULL) return;
            screen_change = paintGameScreen(screen_data->game_screen, renderer, game_data, user_data);
            break;
        case PAUSE_SCREEN:
            if (screen_data->pause_screen == NULL) return;
            if (screen_data->pause_screen->do_entrance_animation)
                paintScreen(screen_data->prev_screen, screen_data, renderer, game_data, user_data);
            else if (screen_data->pause_screen->do_exit_animation)
                paintScreen(screen_data->pause_screen->next_screen, screen_data, renderer, game_data, user_data);
            screen_change = paintPauseScreen(screen_data->pause_screen, renderer);
            break;
        case SETTINGS_SCREEN:
            if (screen_data->settings_screen == NULL) return;
            if (screen_data->settings_screen->do_entrance_animation || screen_data->settings_screen->do_exit_animation)
                paintScreen(screen_data->settings_screen->prev_screen, screen_data, renderer, game_data, user_data);
            screen_change = paintSettingsScreen(screen_data->settings_screen, renderer);
            break;
        default:
            break;
    }
    changeScreen(screen_data, screen_change, renderer, user_data);
}

/*
 * event handling
 */

void mousePressed(const ScreenData* screen_data, const SDL_FPoint pt) {
    switch (screen_data->current_screen) {
        case START_SCREEN:
            startScreenMousePressed(screen_data->start_screen, pt);
            break;
        case SAVES_SCREEN:
            savesScreenMousePressed(screen_data->saves_screen, pt);
            break;
        case GAME_SCREEN:
            gameScreenMousePressed(screen_data->game_screen, pt);
            break;
        case PAUSE_SCREEN:
            pauseScreenMousePressed(screen_data->pause_screen, pt);
            break;
        case SETTINGS_SCREEN:
            settingsScreenMousePressed(screen_data->settings_screen, pt);
            break;
        default:
            break;
    }
}

void mouseReleased(ScreenData* screen_data, const SDL_FPoint pt, SDL_Renderer* renderer, bool* program_running,
        GameData** game_data, UserData* user_data, SDL_Window* window) {
    int screen_change = -1;
    switch (screen_data->current_screen) {
        case START_SCREEN:
            screen_change = startScreenMouseReleased(screen_data->start_screen, pt, program_running);
            break;
        case SAVES_SCREEN:
            screen_change = savesScreenMouseReleased(screen_data->saves_screen, pt, *game_data);
            break;
        case GAME_SCREEN:
            screen_change = gameScreenMouseReleased(screen_data->game_screen, pt);
            break;
        case PAUSE_SCREEN:
            screen_change = pauseScreenMouseReleased(screen_data->pause_screen, pt, game_data);
            break;
        case SETTINGS_SCREEN:
            screen_change = settingsScreenMouseReleased(screen_data->settings_screen, pt, user_data, window);
            break;
        default:
            break;
    }
    changeScreen(screen_data, screen_change, renderer, user_data);
}

void mouseClicked(const ScreenData* screen_data, const SDL_FPoint pt) {

}

void mouseDragged(const ScreenData* screen_data, const SDL_FPoint pt, double prev_mouse_x, const bool mouse_down,
        const GameData* game_data, UserData* user_data) {
    switch (screen_data->current_screen) {
        case START_SCREEN:
            startScreenMouseDragged(screen_data->start_screen, pt);
            break;
        case SAVES_SCREEN:
            savesScreenMouseDragged(screen_data->saves_screen, pt);
            break;
        case GAME_SCREEN:
            gameScreenMouseMoved(screen_data->game_screen, pt, prev_mouse_x, mouse_down, game_data, user_data);
            break;
        case PAUSE_SCREEN:
            pauseScreenMouseDragged(screen_data->pause_screen, pt);
            break;
        case SETTINGS_SCREEN:
            settingsScreenMouseDragged(screen_data->settings_screen, pt, user_data);
            break;
        default:
            break;
    }
}

void keyPressed(const ScreenData* screen_data, const SDL_Keycode keycode, const GameData* game_data) {
    if (screen_data->current_screen == GAME_SCREEN) {
        gameScreenKeyPressed(keycode, game_data);
    }
}

void keyReleased(const ScreenData* screen_data, const SDL_Keycode keycode, const GameData* game_data) {
    if (screen_data->current_screen == GAME_SCREEN) {
        gameScreenKeyReleased(keycode, game_data);
    }
}

void keyClicked(ScreenData* screen_data, const SDL_Keycode keycode, SDL_Renderer* renderer, const UserData* user_data) {
    int screen_change = -1;
    if (screen_data->current_screen == GAME_SCREEN) {
        screen_change = gameScreenKeyClicked(keycode);
    }
    changeScreen(screen_data, screen_change, renderer, user_data);
}