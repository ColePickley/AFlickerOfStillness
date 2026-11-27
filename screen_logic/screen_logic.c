#include "screen_logic.h"

#include <stdio.h>
#include <stdlib.h>
#include "../file_handling/save_handling.h"
#include "../file_handling/map_reader.h"
#include "loading_overlay_painter.h"

void initScreenData(ScreenData** screen_data, SDL_Renderer* renderer, const UserData* user_data) {
    *screen_data = malloc(sizeof(ScreenData));
    (*screen_data)->current_screen = START_SCREEN;
    (*screen_data)->prev_screen = START_SCREEN;
    (*screen_data)->next_screen = NO_CHANGE;
    initStartScreen(&(*screen_data)->start_screen);
    setStartScreenTextures((*screen_data)->start_screen, renderer);
    initSavesScreen(&(*screen_data)->saves_screen);
    setSavesScreenTextures((*screen_data)->saves_screen, renderer);
    initSettingsScreen(&(*screen_data)->settings_screen, user_data, (*screen_data)->prev_screen);
    setSettingsScreenTextures((*screen_data)->settings_screen, renderer);
    (*screen_data)->pause_screen = NULL;
    (*screen_data)->game_screen = NULL;
    (*screen_data)->death_screen = NULL;
    (*screen_data)->do_exit_animation = false;
    (*screen_data)->is_loading_game = false;

    (*screen_data)->is_loading_in_background = false;
    (*screen_data)->active_async_task = NULL;
    (*screen_data)->bg_thread = NULL;
}

void destroyScreenData(ScreenData** screen_data) {
    if (*screen_data == NULL) return;

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
    destroyDeathScreen(&(*screen_data)->death_screen);
    free(*screen_data);
    *screen_data = NULL;
}

static int asyncScreenChange(void* data) {
    ThreadLoadingData* args = data;
    if (args->screen_data->current_screen == START_SCREEN) {
        if (args->screen_data->next_screen == SAVES_SCREEN) {
            initGameScreen(&args->screen_data->game_screen, args->game_data);
        }
    } else if (args->screen_data->current_screen == SAVES_SCREEN) {
        if (args->screen_data->next_screen == START_SCREEN) {
            initSettingsScreen(&args->screen_data->settings_screen, args->user_data, args->screen_data->next_screen);
        } else if (args->screen_data->next_screen == GAME_SCREEN) {
            args->game_data->level_index = getLevelIndex(args->game_data->save_index);
            readMap(args->game_data);
            loadSave(args->game_data);
            writeSave(args->game_data);
            initDeathScreen(&args->screen_data->death_screen);
            initPauseScreen(&args->screen_data->pause_screen);
        }
    } else if (args->screen_data->current_screen == GAME_SCREEN) {
        if (args->screen_data->next_screen == PAUSE_SCREEN || args->screen_data->next_screen == DEATH_SCREEN) {
            initSettingsScreen(&args->screen_data->settings_screen, args->user_data, args->screen_data->next_screen);
            initStartScreen(&args->screen_data->start_screen);
        }
    } else if (args->screen_data->current_screen == PAUSE_SCREEN) {
        if (args->screen_data->next_screen == START_SCREEN) {
            initSavesScreen(&args->screen_data->saves_screen);
        } else if (args->screen_data->next_screen == GAME_SCREEN) {
            initDeathScreen(&args->screen_data->death_screen);
        }
    } else if (args->screen_data->current_screen == DEATH_SCREEN) {
        if (args->screen_data->next_screen == START_SCREEN) {
            initSavesScreen(&args->screen_data->saves_screen);
        } else if (args->screen_data->next_screen == GAME_SCREEN) {
            initPauseScreen(&args->screen_data->pause_screen);
        }
    }
    SDL_SetAtomicInt(&args->is_done, 1);
    return 0;
}

void changeScreen(ScreenData* screen_data, const ScreenChange screen_change, SDL_Renderer* renderer,
        GameData* game_data, const UserData* user_data, SDL_Window* window) {
    if (screen_change == NO_CHANGE) return;
    if (screen_data->is_loading_in_background) return;
    if (screen_change == ANIMATION_FINISHED) {
        loadScreenTextures(screen_data, renderer, window, game_data);
        screen_data->prev_screen = screen_data->current_screen;
        screen_data->current_screen = screen_data->next_screen;
        screen_data->next_screen = NO_CHANGE;
        screen_data->do_exit_animation = false;
        return;
    }
    if (screen_change == GAME_SCREEN && screen_data->current_screen == SAVES_SCREEN) {
        screen_data->is_loading_game = true;
    }
    screen_data->next_screen = screen_change;
    screen_data->is_loading_in_background = true;
    ThreadLoadingData* args = malloc(sizeof(ThreadLoadingData));
    args->screen_data = screen_data;
    args->game_data = game_data;
    args->user_data = user_data;
    args->renderer = renderer;
    SDL_SetAtomicInt(&args->is_done, 0);
    screen_data->active_async_task = args;
    screen_data->bg_thread = SDL_CreateThread(asyncScreenChange, "GameLoadingWorker", args);
    if (!screen_data->bg_thread) {
        SDL_Log("Thread creation failed! Falling back to synchronous safety fallback.");
    }
}

void loadScreenTextures(ScreenData* screen_data, SDL_Renderer* renderer, SDL_Window* window,
        const GameData* game_data) {
    if (screen_data->current_screen == START_SCREEN) {
        if (screen_data->next_screen == SAVES_SCREEN) {
            destroySettingsScreen(&screen_data->settings_screen);
            setGameScreenTextures(screen_data->game_screen, renderer);
        }
    } else if (screen_data->current_screen == SAVES_SCREEN) {
        if (screen_data->next_screen == START_SCREEN) {
            destroyGameScreen(&screen_data->game_screen);
            setSettingsScreenTextures(screen_data->settings_screen, renderer);
        } else if (screen_data->next_screen == GAME_SCREEN) {
            resetEntityClocks(game_data);
            destroyStartScreen(&screen_data->start_screen);
            destroySavesScreen(&screen_data->saves_screen);
            setPauseScreenTextures(screen_data->pause_screen, renderer);
            if (!SDL_SetWindowRelativeMouseMode(window, true)) {
                SDL_Log("Could not enable relative mouse mode: %s", SDL_GetError());
            }
        }
    } else if (screen_data->current_screen == GAME_SCREEN) {
        if (screen_data->next_screen == PAUSE_SCREEN) {
            destroyDeathScreen(&screen_data->death_screen);
            setSettingsScreenTextures(screen_data->settings_screen, renderer);
            setStartScreenTextures(screen_data->start_screen, renderer);
        } else if (screen_data->next_screen == DEATH_SCREEN) {
            destroyPauseScreen(&screen_data->pause_screen);
            setSettingsScreenTextures(screen_data->settings_screen, renderer);
            setStartScreenTextures(screen_data->start_screen, renderer);
        }
        if (!SDL_SetWindowRelativeMouseMode(window, false)) {
            SDL_Log("Could not enable relative mouse mode: %s", SDL_GetError());
        }
    } else if (screen_data->current_screen == PAUSE_SCREEN) {
        if (screen_data->next_screen == GAME_SCREEN) {
            resetEntityClocks(game_data);
            destroySettingsScreen(&screen_data->settings_screen);
            destroyStartScreen(&screen_data->start_screen);
            if (!SDL_SetWindowRelativeMouseMode(window, true)) {
                SDL_Log("Could not enable relative mouse mode: %s", SDL_GetError());
            }
        } else if (screen_data->next_screen == START_SCREEN) {
            destroyPauseScreen(&screen_data->pause_screen);
            destroyGameScreen(&screen_data->game_screen);
            setSavesScreenTextures(screen_data->saves_screen, renderer);
            screen_data->settings_screen->prev_screen_change = START_SCREEN;
        }
    } else if (screen_data->current_screen == DEATH_SCREEN) {
        if (screen_data->next_screen == GAME_SCREEN) {
            resetEntityClocks(game_data);
            destroySettingsScreen(&screen_data->settings_screen);
            destroyStartScreen(&screen_data->start_screen);
            setPauseScreenTextures(screen_data->pause_screen, renderer);
            if (!SDL_SetWindowRelativeMouseMode(window, true)) {
                SDL_Log("Could not enable relative mouse mode: %s", SDL_GetError());
            }
        } else if (screen_data->next_screen == START_SCREEN) {
            destroyDeathScreen(&screen_data->death_screen);
            destroyGameScreen(&screen_data->game_screen);
            setSavesScreenTextures(screen_data->saves_screen, renderer);
            screen_data->settings_screen->prev_screen_change = START_SCREEN;
        }
    }
}

void paintScreen(const ScreenChange screen_change, const bool is_recursive, ScreenData* screen_data,
        SDL_Renderer* renderer, GameData* game_data, const UserData* user_data, SDL_Window* window) {
    bool paint_loading_overlay = false;
    if (screen_data->is_loading_in_background && screen_data->active_async_task && !is_recursive) {
        if (SDL_GetAtomicInt(&screen_data->active_async_task->is_done)) {
            SDL_WaitThread(screen_data->bg_thread, NULL);
            if (screen_data->current_screen != PAUSE_SCREEN && screen_data->current_screen != DEATH_SCREEN) {
                loadScreenTextures(screen_data, renderer, window, game_data);
                screen_data->prev_screen = screen_data->current_screen;
                screen_data->current_screen = screen_data->next_screen;
                screen_data->next_screen = NO_CHANGE;
            } else {
                screen_data->do_exit_animation = true;
            }
            free(screen_data->active_async_task);
            screen_data->active_async_task = NULL;
            screen_data->bg_thread = NULL;
            screen_data->is_loading_in_background = false;
            resetLoadingOverlay();
        } else if (screen_change == SAVES_SCREEN && screen_data->is_loading_game) {
            paint_loading_overlay = true;
        }
    }

    // clear screen
    if (!is_recursive) {
        SDL_SetRenderDrawColor(renderer, 38, 31, 28, 255);
        SDL_RenderClear(renderer);
    }

    ScreenChange screen_change_out = NO_CHANGE;
    switch (screen_change) {
        case START_SCREEN:
            if (screen_data->start_screen == NULL) return;
            screen_change_out = paintStartScreen(screen_data->start_screen, renderer);
            break;
        case SAVES_SCREEN:
            if (screen_data->saves_screen == NULL) return;
            screen_change_out = paintSavesScreen(screen_data->saves_screen, renderer);
            break;
        case GAME_SCREEN:
            if (screen_data->game_screen == NULL) return;
            screen_change_out = paintGameScreen(screen_data->game_screen, renderer, game_data, user_data);
            break;
        case PAUSE_SCREEN:
            if (screen_data->pause_screen == NULL) return;
            if (screen_data->pause_screen->do_entrance_animation) {
                paintScreen(screen_data->prev_screen, true, screen_data, renderer, game_data, user_data,
                    window);
            } else if (screen_data->do_exit_animation) {
                paintScreen(screen_data->next_screen, true, screen_data, renderer, game_data,
                    user_data, window);
            }
            screen_change_out = paintPauseScreen(screen_data->pause_screen, renderer, &screen_data->do_exit_animation);
            break;
        case SETTINGS_SCREEN:
            if (screen_data->settings_screen == NULL) return;
            if (screen_data->settings_screen->do_entrance_animation ||
                    screen_data->settings_screen->do_exit_animation) {
                paintScreen(screen_data->settings_screen->prev_screen_change, true, screen_data, renderer, game_data,
                    user_data, window);
            }
            screen_change_out = paintSettingsScreen(screen_data->settings_screen, renderer);
            break;
        case DEATH_SCREEN:
            if (screen_data->death_screen == NULL) return;
            if (screen_data->death_screen->do_entrance_animation)
                paintScreen(screen_data->prev_screen, true, screen_data, renderer, game_data, user_data,
                    window);
            if (screen_data->do_exit_animation) {
                paintScreen(screen_data->next_screen, true, screen_data, renderer, game_data,
                    user_data, window);
            }
            screen_change_out = paintDeathScreen(screen_data->death_screen, renderer, &screen_data->do_exit_animation);
        default:
            break;
    }
    if (paint_loading_overlay) {
        paintLoadingOverlay(renderer);
    }
    if (!is_recursive) changeScreen(screen_data, screen_change_out, renderer, game_data, user_data, window);
}

/*
 *----------------------------------------------------------------------------------------------------------------------
 * Event handling
 *----------------------------------------------------------------------------------------------------------------------
 */

void mousePressed(const ScreenData* screen_data, const SDL_FPoint pt, GameData* game_data) {
    switch (screen_data->current_screen) {
        case START_SCREEN:
            startScreenMousePressed(screen_data->start_screen, pt);
            break;
        case SAVES_SCREEN:
            savesScreenMousePressed(screen_data->saves_screen, pt);
            break;
        case GAME_SCREEN:
            gameScreenMousePressed(screen_data->game_screen, pt, game_data);
            break;
        case PAUSE_SCREEN:
            pauseScreenMousePressed(screen_data->pause_screen, pt);
            break;
        case SETTINGS_SCREEN:
            settingsScreenMousePressed(screen_data->settings_screen, pt);
            break;
        case DEATH_SCREEN:
            deathScreenMousePressed(screen_data->death_screen, pt);
            break;
        default:
            break;
    }
}

void mouseReleased(ScreenData* screen_data, const SDL_FPoint pt, SDL_Renderer* renderer, bool* program_running,
        GameData** game_data, UserData* user_data, SDL_Window* window) {
    ScreenChange screen_change = NO_CHANGE;
    switch (screen_data->current_screen) {
        case START_SCREEN:
            screen_change = startScreenMouseReleased(screen_data->start_screen, pt, program_running);
            break;
        case SAVES_SCREEN:
            screen_change = savesScreenMouseReleased(screen_data->saves_screen, pt, *game_data);
            break;
        case GAME_SCREEN:
            screen_change = gameScreenMouseReleased(screen_data->game_screen, pt, *game_data, window);
            break;
        case PAUSE_SCREEN:
            screen_change = pauseScreenMouseReleased(screen_data->pause_screen, pt, game_data);
            break;
        case SETTINGS_SCREEN:
            screen_change = settingsScreenMouseReleased(screen_data->settings_screen, pt, user_data, window);
            break;
        case DEATH_SCREEN:
            screen_change = deathScreenMouseReleased(screen_data->death_screen, pt, game_data);
            break;
        default:
            break;
    }
    changeScreen(screen_data, screen_change, renderer, *game_data, user_data, window);
}

void mouseClicked(const ScreenData* screen_data, const SDL_FPoint pt, GameData* game_data) {
    switch (screen_data->current_screen) {
        case GAME_SCREEN:
            gameScreenMouseClicked(screen_data->game_screen, pt, game_data);
            break;
        default:
            break;
    }
}

void mouseDragged(const ScreenData* screen_data, const SDL_FPoint screen_pt, const GameData* game_data,
        UserData* user_data) {
    switch (screen_data->current_screen) {
        case START_SCREEN:
            startScreenMouseDragged(screen_data->start_screen, screen_pt);
            break;
        case SAVES_SCREEN:
            savesScreenMouseDragged(screen_data->saves_screen, screen_pt);
            break;
        case GAME_SCREEN:
            gameScreenMouseDragged(screen_data->game_screen, screen_pt, game_data, user_data);
            break;
        case PAUSE_SCREEN:
            pauseScreenMouseDragged(screen_data->pause_screen, screen_pt);
            break;
        case SETTINGS_SCREEN:
            settingsScreenMouseDragged(screen_data->settings_screen, screen_pt, user_data);
            break;
        case DEATH_SCREEN:
            deathScreenMouseDragged(screen_data->death_screen, screen_pt);
            break;
        default:
            break;
    }
}

void mouseMoved(const ScreenData* screen_data, const SDL_FPoint pt, const GameData* game_data,
        const UserData* user_data) {
    switch (screen_data->current_screen) {
        case START_SCREEN:
            startScreenMouseMoved(screen_data->start_screen, pt);
            break;
        case SAVES_SCREEN:
            savesScreenMouseMoved(screen_data->saves_screen, pt);
            break;
        case GAME_SCREEN:
            gameScreenMouseMoved(screen_data->game_screen, pt, game_data, user_data);
            break;
        case PAUSE_SCREEN:
            pauseScreenMouseMoved(screen_data->pause_screen, pt);
            break;
        case SETTINGS_SCREEN:
            settingsScreenMouseMoved(screen_data->settings_screen, pt);
            break;
        case DEATH_SCREEN:
            deathScreenMouseMoved(screen_data->death_screen, pt);
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

void keyClicked(ScreenData* screen_data, const SDL_Keycode keycode, SDL_Renderer* renderer, GameData* game_data,
        const UserData* user_data, SDL_Window* window) {
    ScreenChange screen_change = NO_CHANGE;
    if (screen_data->current_screen == GAME_SCREEN) {
        screen_change = gameScreenKeyClicked(keycode, game_data);
    }
    changeScreen(screen_data, screen_change, renderer, game_data, user_data, window);
}