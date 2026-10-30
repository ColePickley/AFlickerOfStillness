#ifndef AFLICKEROFSTILLNESS_SCREEN_LOGIC_H
#define AFLICKEROFSTILLNESS_SCREEN_LOGIC_H
#include "death_screen.h"
#include "game_screen.h"
#include "saves_screen.h"
#include "start_screen.h"
#include "pause_screen.h"
#include "settings_screen.h"

typedef struct ScreenData ScreenData;

typedef struct {
    ScreenData* screen_data;
    GameData* game_data;
    const UserData* user_data;
    SDL_Renderer* renderer;
    int screen_id;
    volatile bool is_done;
} ThreadLoadingData;

struct ScreenData {
    uint8_t prev_screen, current_screen;
    StartScreen* start_screen;
    SavesScreen* saves_screen;
    GameScreen* game_screen;
    PauseScreen* pause_screen;
    SettingsScreen* settings_screen;
    DeathScreen* death_screen;

    bool is_loading_in_background;
    ThreadLoadingData* active_async_task;
    SDL_Thread* bg_thread;
};

void initScreenData(ScreenData** screen_data, SDL_Renderer* renderer, const UserData* user_data);

void destroyScreenData(ScreenData** screen_data);

void loadScreenTextures(ScreenData* screen_data, SDL_Renderer* renderer, int screen_id, SDL_Window* window);

static int asyncScreenChange(void* data);

void changeScreen(ScreenData* screen_data, int screen_id, SDL_Renderer* renderer, GameData* game_data,
    const UserData* user_data);

void paintScreen(int screen_id, ScreenData* screen_data, SDL_Renderer* renderer, GameData* game_data,
    const UserData* user_data, SDL_Window* window);

/*
 *----------------------------------------------------------------------------------------------------------------------
 * Event handling
 *----------------------------------------------------------------------------------------------------------------------
 */

void mousePressed(const ScreenData* screen_data, SDL_FPoint pt, GameData* game_data);

void mouseReleased(ScreenData* screen_data, SDL_FPoint pt, SDL_Renderer* renderer, bool* program_running,
    GameData** game_data, UserData* user_data, SDL_Window* window);

void mouseClicked(const ScreenData* screen_data, SDL_FPoint pt, GameData* game_data);

void mouseDragged(const ScreenData* screen_data, SDL_FPoint pt, const GameData* game_data, UserData* user_data);

void mouseMoved(const ScreenData* screen_data, SDL_FPoint pt, const GameData* game_data, const UserData* user_data);

void keyPressed(const ScreenData* screen_data, SDL_Keycode keycode, const GameData* game_data);

void keyReleased(const ScreenData* screen_data, SDL_Keycode keycode, const GameData* game_data);

void keyClicked(ScreenData* screen_data, SDL_Keycode keycode, SDL_Renderer* renderer, GameData* game_data,
    const UserData* user_data);

#endif //AFLICKEROFSTILLNESS_SCREEN_LOGIC_H
