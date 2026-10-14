#ifndef AFLICKEROFSTILLNESS_SAVES_SCREEN_H
#define AFLICKEROFSTILLNESS_SAVES_SCREEN_H
#include <SDL3/SDL_render.h>

#include "../program_data.h"
#include "button.h"
#include "label.h"

typedef struct {
    SDL_Texture* background_texture;
    SDL_Texture* left_panel_texture;
    SDL_Texture* right_panel_texture;
    SDL_Texture* delete_save_texture;
}SavesScreenTextures;

typedef struct {
    SDL_Surface* background_surface;
    SDL_Surface* left_panel_surface;
    SDL_Surface* right_panel_surface;
    SDL_Surface* delete_save_surface;
}SavesScreenSurfaces;

typedef struct {
    SavesScreenTextures* textures;
    SavesScreenSurfaces* surfaces;
    SDL_FRect left_panel_rect;
    SDL_FRect right_panel_rect;
    Label* left_panel_label;
    Label* right_panel_label;
    Label* select_save_label;
    Label* saves_label;
    Button* back_button;
    Button* play_button;
    Button* save_buttons[3];
    Button* delete_save_button;
}SavesScreen;

void initSavesScreen(SavesScreen** saves_screen);

void setSavesScreenTextures(const SavesScreen* saves_screen, SDL_Renderer* renderer);

void destroySavesScreen(SavesScreen** saves_screen);

int paintSavesScreen(const SavesScreen* saves_screen, SDL_Renderer* renderer);

/*
 * Event handling
 */

void savesScreenMousePressed(const SavesScreen* saves_screen, SDL_FPoint mouse_pos);

int savesScreenMouseReleased(SavesScreen* saves_screen, SDL_FPoint mouse_pos, GameData* game_data);

void savesScreenMouseDragged(const SavesScreen* saves_screen, SDL_FPoint mouse_pos);

/*
 * Helper functions
 */

static void setSavesScreenSurfaces(const SavesScreen* saves_screen);

static void setSavesScreenLabels(SavesScreen* saves_screen);

static void setSavesScreenButtons(SavesScreen* saves_screen);

static void setSavesScreenRects(SavesScreen* saves_screen);

#endif //AFLICKEROFSTILLNESS_SAVES_SCREEN_H
