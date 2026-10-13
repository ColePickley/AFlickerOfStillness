#include "screen_data.h"

void initScreenData(ScreenData** screen_data, SDL_Renderer* renderer, const UserData* user_data) {
    *screen_data = malloc(sizeof(ScreenData));
    (*screen_data)->current_screen = START_SCREEN;
    (*screen_data)->prev_screen = START_SCREEN;
    loadScreen(*screen_data, START_SCREEN, renderer, user_data);
    loadScreen(*screen_data, SAVES_SCREEN, renderer, user_data);
    loadScreen(*screen_data, SETTINGS_SCREEN, renderer, user_data);
    (*screen_data)->pause_screen = NULL;
    (*screen_data)->game_screen = NULL;
}

void destroyScreenData(ScreenData** screen_data) {
    destroyStartScreen(&(*screen_data)->start_screen);
    destroySavesScreen(&(*screen_data)->saves_screen);
    destroyGameScreen(&(*screen_data)->game_screen);
    destroyPauseScreen(&(*screen_data)->pause_screen);
    destroySettingsScreen(&(*screen_data)->settings_screen);
    free(*screen_data);
    *screen_data = NULL;
}

void loadScreen(ScreenData* screen_data, const uint8_t new_screen, SDL_Renderer* renderer, const UserData* user_data) {
    switch (new_screen) {
        case START_SCREEN:
            initStartScreen(&screen_data->start_screen, renderer, user_data);
            break;
        case SAVES_SCREEN:
            initSavesScreen(&screen_data->saves_screen, renderer);
            break;
        case GAME_SCREEN:
            initGameScreen(&screen_data->game_screen, renderer);
            break;
        case PAUSE_SCREEN:
            initPauseScreen(&screen_data->pause_screen, renderer);
            break;
        case SETTINGS_SCREEN:
            initSettingsScreen(&screen_data->settings_screen, renderer, user_data, screen_data->current_screen);
            break;
        default:
            break;
    }
}

void unloadScreen(ScreenData* screen_data, const uint8_t screen_id) {
    switch (screen_id) {
        case START_SCREEN:
            destroyStartScreen(&screen_data->start_screen);
            break;
        case SAVES_SCREEN:
            destroySavesScreen(&screen_data->saves_screen);
            break;
        case GAME_SCREEN:
            destroyGameScreen(&screen_data->game_screen);
            break;
        case PAUSE_SCREEN:
            destroyPauseScreen(&screen_data->pause_screen);
            break;
        case SETTINGS_SCREEN:
            destroySettingsScreen(&screen_data->settings_screen);
            break;
        default:
            break;
    }
}

void changeScreen(ScreenData* screen_data, const int screen_id, SDL_Renderer* renderer, const UserData* user_data) {
    if (screen_id == -1) return;
    screen_data->prev_screen = screen_data->current_screen;
    screen_data->current_screen = screen_id;
    if (screen_data->prev_screen == START_SCREEN) {
        if (screen_id == SAVES_SCREEN) {
            unloadScreen(screen_data, SETTINGS_SCREEN);
            loadScreen(screen_data, GAME_SCREEN, renderer, user_data);
        } else if (screen_id == SETTINGS_SCREEN) {
            unloadScreen(screen_data, SAVES_SCREEN);
        }
    } else if (screen_data->prev_screen == SAVES_SCREEN) {
        if (screen_id == START_SCREEN) {
            unloadScreen(screen_data, GAME_SCREEN);
            loadScreen(screen_data, SETTINGS_SCREEN, renderer, user_data);
        } else if (screen_id == GAME_SCREEN) {
            unloadScreen(screen_data, START_SCREEN);
            unloadScreen(screen_data, SAVES_SCREEN);
            loadScreen(screen_data, PAUSE_SCREEN, renderer, user_data);
        }
    } else if (screen_data->prev_screen == SETTINGS_SCREEN) {
        if (screen_id == START_SCREEN) {
            loadScreen(screen_data, SAVES_SCREEN, renderer, user_data);
        } else if (screen_id == PAUSE_SCREEN) {
            loadScreen(screen_data, GAME_SCREEN, renderer, user_data);
            loadScreen(screen_data, START_SCREEN, renderer, user_data);
        }
    } else if (screen_data->prev_screen == GAME_SCREEN) {
        if (screen_id == PAUSE_SCREEN) {
            loadScreen(screen_data, SETTINGS_SCREEN, renderer, user_data);
            loadScreen(screen_data, START_SCREEN, renderer, user_data);
        }
    } else if (screen_data->prev_screen == PAUSE_SCREEN) {
        if (screen_id == GAME_SCREEN) {
            unloadScreen(screen_data, SETTINGS_SCREEN);
            unloadScreen(screen_data, START_SCREEN);
        } else if (screen_id == SETTINGS_SCREEN) {
            unloadScreen(screen_data, GAME_SCREEN);
            unloadScreen(screen_data, START_SCREEN);
        } else if (screen_id == START_SCREEN) {
            unloadScreen(screen_data, PAUSE_SCREEN);
            unloadScreen(screen_data, GAME_SCREEN);
            loadScreen(screen_data, SAVES_SCREEN, renderer, user_data);
            screen_data->settings_screen->prev_screen = START_SCREEN;
        }
    }
}

void paintScreen(const int screen_id, ScreenData* screen_data, SDL_Renderer* renderer, GameData* game_data,
        const UserData* user_data) {
    int screen_change = -1;
    switch (screen_id) {
        case START_SCREEN:
            screen_change = paintStartScreen(screen_data->start_screen, renderer);
            break;
        case SAVES_SCREEN:
            screen_change = paintSavesScreen(screen_data->saves_screen, renderer);
            break;
        case GAME_SCREEN:
            screen_change = paintGameScreen(screen_data->game_screen, renderer, game_data, user_data);
            break;
        case PAUSE_SCREEN:
            if (screen_data->pause_screen->do_entrance_animation)
                paintScreen(screen_data->prev_screen, screen_data, renderer, game_data, user_data);
            else if (screen_data->pause_screen->do_exit_animation)
                paintScreen(screen_data->pause_screen->next_screen, screen_data, renderer, game_data, user_data);
            screen_change = paintPauseScreen(screen_data->pause_screen, renderer);
            break;
        case SETTINGS_SCREEN:
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