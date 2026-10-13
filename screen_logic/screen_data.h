#ifndef AFLICKEROFSTILLNESS_SCREEN_DATA_H
#define AFLICKEROFSTILLNESS_SCREEN_DATA_H
#include "game_screen.h"
#include "saves_screen.h"
#include "start_screen.h"
#include "pause_screen.h"
#include "settings_screen.h"

typedef struct {
    uint8_t prev_screen, current_screen;
    StartScreen* start_screen;
    SavesScreen* saves_screen;
    GameScreen* game_screen;
    PauseScreen* pause_screen;
    SettingsScreen* settings_screen;
}ScreenData;

void initScreenData(ScreenData** screen_data, SDL_Renderer* renderer, const UserData* user_data);

void destroyScreenData(ScreenData** screen_data);

void loadScreen(ScreenData* screen_data, uint8_t new_screen, SDL_Renderer* renderer, const UserData* user_data);

void unloadScreen(ScreenData* screen_data, uint8_t screen_id);

void changeScreen(ScreenData* screen_data, int screen_id, SDL_Renderer* renderer, const UserData* user_data);

void paintScreen(int screen_id, ScreenData* screen_data, SDL_Renderer* renderer, GameData* game_data,
    const UserData* user_data);

/*
 * Event handling
 */

void mousePressed(const ScreenData* screen_data, SDL_FPoint pt);

void mouseReleased(ScreenData* screen_data, SDL_FPoint pt, SDL_Renderer* renderer, bool* program_running,
    GameData** game_data, UserData* user_data, SDL_Window* window);

void mouseClicked(const ScreenData* screen_data, SDL_FPoint pt);

void mouseDragged(const ScreenData* screen_data, SDL_FPoint pt, double prev_mouse_x, bool mouse_down,
    const GameData* game_data, UserData* user_data);

void keyPressed(const ScreenData* screen_data, SDL_Keycode keycode, const GameData* game_data);

void keyReleased(const ScreenData* screen_data, SDL_Keycode keycode, const GameData* game_data);

void keyClicked(ScreenData* screen_data, SDL_Keycode keycode, SDL_Renderer* renderer, const UserData* user_data);

#endif //AFLICKEROFSTILLNESS_SCREEN_DATA_H
