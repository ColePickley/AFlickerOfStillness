#ifndef AFLICKEROFSTILLNESS_START_SCREEN_H
#define AFLICKEROFSTILLNESS_START_SCREEN_H
#include <SDL3/SDL_render.h>

#include "button.h"
#include "label.h"
#include "settings_screen.h"

typedef struct {
    bool paint_start_screen;
    bool paint_settings_screen;
    SettingsScreen* settings_screen;
    SDL_Texture* background_texture;
    SDL_Texture* left_panel_texture;
    SDL_Texture* right_panel_texture;
    SDL_FRect left_panel_rect;
    SDL_FRect right_panel_rect;
    Label* left_panel_label;
    Label* right_panel_label;
    Label* game_label;
    Button* play_button;
    Button* settings_button;
    Button* exit_button;
}StartScreen;

void initStartScreen(StartScreen* start_screen, SDL_Renderer* renderer, const UserData* user_data);

void paintStartScreen(StartScreen* start_screen, SDL_Renderer* renderer);

void startScreenMousePressed(const StartScreen* start_screen, SDL_FPoint mouse_pos);

void startScreenMouseReleased(StartScreen* start_screen, SDL_FPoint mouse_pos, bool* program_running,
    uint8_t* current_screen, uint8_t* prev_screen, UserData* user_data, SDL_Window* window);

void startScreenMouseDragged(const StartScreen* start_screen, SDL_FPoint mouse_pos, UserData* user_data);

void destroyStartScreen(const StartScreen* start_screen);

static void setStartScreenBackground(StartScreen* start_screen, SDL_Renderer* renderer);

static void setStartScreenLabels(StartScreen* start_screen);

static void setStartScreenButtons(StartScreen* start_screen);

#endif //AFLICKEROFSTILLNESS_START_SCREEN_H
