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
    SDL_Texture* delete_save_texture;
}SavesScreen;

void initSavesScreen(SavesScreen* saves_screen, SDL_Renderer* renderer);

void destroySavesScreen(const SavesScreen* saves_screen);

void paintSavesScreen(const SavesScreen* saves_screen, SDL_Renderer* renderer);

void savesScreenMousePressed(const SavesScreen* saves_screen, SDL_FPoint mouse_pos);

void savesScreenMouseReleased(SavesScreen* saves_screen, SDL_FPoint mouse_pos, GameData* game_data,
    uint8_t* current_screen, uint8_t* prev_screen);

void savesScreenMouseDragged(const SavesScreen* saves_screen, SDL_FPoint mouse_pos);

static void setSavesScreenBackground(SavesScreen* saves_screen, SDL_Renderer* renderer);

static void setSavesScreenLabels(SavesScreen* saves_screen);

static void setSavesScreenButtons(SavesScreen* saves_screen);

static void setSavesScreenDeleteSaveTexture(SavesScreen* saves_screen, SDL_Renderer* renderer);

#endif //AFLICKEROFSTILLNESS_SAVES_SCREEN_H
