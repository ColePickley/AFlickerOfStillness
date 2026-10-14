#ifndef AFLICKEROFSTILLNESS_SETTINGS_SCREEN_H
#define AFLICKEROFSTILLNESS_SETTINGS_SCREEN_H
#include "button.h"
#include "label.h"
#include "../file_handling/user_data_handling.h"

typedef struct {
    SDL_Texture* left_background_texture;
    SDL_Texture* right_background_texture;
}SettingsScreenTextures;

typedef struct {
    SDL_Surface* left_background_surface;
    SDL_Surface* right_background_surface;
}SettingsScreenSurfaces;

typedef struct {
    SettingsScreenTextures* textures;
    SettingsScreenSurfaces* surfaces;
    SDL_FRect left_background_rect;
    SDL_FRect right_background_rect;
    SDL_FRect fps_slider_rects[5];
    SDL_FRect graphics_quality_slider_rects[5];
    SDL_FRect sensitivity_slider_rect;
    Label* settings_label;
    Label* fps_label;
    Label* graphics_quality_label;
    Label* sensitivity_label;
    Button* fullscreen_button;
    Button* fps_button;
    Button* graphics_quality_button;
    Button* sensitivity_button;
    Button* back_button;
    Button* reset_button;
    Button* save_button;
    uint64_t last_paint_time;
    bool do_entrance_animation;
    bool do_exit_animation;
    int prev_screen;
}SettingsScreen;

void initSettingsScreen(SettingsScreen** settings_screen, const UserData* user_data, int prev_screen);

void setSettingsScreenTextures(const SettingsScreen* settings_screen, SDL_Renderer* renderer);

void resetSettingsScreen(SettingsScreen* settings_screen, const UserData* user_data);

void destroySettingsScreen(SettingsScreen** settings_screen);

int paintSettingsScreen(SettingsScreen* settings_screen, SDL_Renderer* renderer);

/*
 * Event Handling
 */

void settingsScreenMousePressed(const SettingsScreen* settings_screen, SDL_FPoint mouse_pos);

int settingsScreenMouseReleased(SettingsScreen* settings_screen, SDL_FPoint mouse_pos, UserData* user_data,
    SDL_Window* window);

void settingsScreenMouseDragged(SettingsScreen* settings_screen, SDL_FPoint mouse_pos, UserData* user_data);

/*
 * Helper Functions
 */

static void setSettingsScreenSurfaces(const SettingsScreen* settings_screen);

static void setSettingsScreenLabels(SettingsScreen* settings_screen, const UserData* user_data);

static void setSettingsScreenButtons(SettingsScreen* settings_screen, const UserData* user_data);

static void setSettingsScreenRects(SettingsScreen* settings_screen);

#endif //AFLICKEROFSTILLNESS_SETTINGS_SCREEN_H
