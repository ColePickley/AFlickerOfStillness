#ifndef AFLICKEROFSTILLNESS_START_SCREEN_H
#define AFLICKEROFSTILLNESS_START_SCREEN_H
#include <SDL3/SDL_render.h>

#include "button.h"
#include "label.h"
#include "settings_screen.h"

typedef struct {
    SDL_Texture* background_texture;
    SDL_Texture* left_panel_texture;
    SDL_Texture* right_panel_texture;
}StartScreenTextures;

typedef struct {
    SDL_Surface* background_surface;
    SDL_Surface* left_panel_surface;
    SDL_Surface* right_panel_surface;
}StartScreenSurfaces;

typedef struct {
    StartScreenTextures* textures;
    StartScreenSurfaces* surfaces;
    SDL_FRect left_panel_rect;
    SDL_FRect right_panel_rect;
    Label* left_panel_label;
    Label* right_panel_label;
    Label* game_label;
    Button* play_button;
    Button* settings_button;
    Button* exit_button;
}StartScreen;

void initStartScreen(StartScreen** start_screen);

void setStartScreenTextures(const StartScreen* start_screen, SDL_Renderer* renderer);

void destroyStartScreen(StartScreen** start_screen);

int paintStartScreen(const StartScreen* start_screen, SDL_Renderer* renderer);

/*
 * Event handling
 */

void startScreenMousePressed(const StartScreen* start_screen, SDL_FPoint mouse_pos);

int startScreenMouseReleased(const StartScreen* start_screen, SDL_FPoint mouse_pos, bool* program_running);

void startScreenMouseDragged(const StartScreen* start_screen, SDL_FPoint mouse_pos);

void startScreenMouseMoved(const StartScreen* start_screen, SDL_FPoint mouse_pos);

/*
 * Helper Functions
 */

static void setStartScreenSurfaces(const StartScreen* start_screen);

static void setStartScreenRects(StartScreen* start_screen);

static void setStartScreenLabels(StartScreen* start_screen);

static void setStartScreenButtons(StartScreen* start_screen);

#endif //AFLICKEROFSTILLNESS_START_SCREEN_H
