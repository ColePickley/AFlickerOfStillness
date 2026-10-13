#ifndef AFLICKEROFSTILLNESS_PAUSE_SCREEN_H
#define AFLICKEROFSTILLNESS_PAUSE_SCREEN_H
#include "settings_screen.h"

#define BACKGROUND_TEXTURE_WIDTH 482.0f
#define BACKGROUND_TEXTURE_HEIGHT 271.125f

typedef struct {
    bool do_entrance_animation;
    bool do_exit_animation;
    int next_screen;
    float animation_state;
    float animation_speed;
    uint64_t last_paint_time;
    SDL_Texture* background_textures[24];
    SDL_Texture* texture_set_1[12];
    SDL_Texture* texture_set_2[12];
    Button* resume_button;
    Button* last_checkpoint_button;
    Button* restart_level_button;
    Button* settings_button;
    Button* exit_button;
}PauseScreen;

void initPauseScreen(PauseScreen** pause_screen, SDL_Renderer* renderer);

void destroyPauseScreen(PauseScreen** pause_screen);

void shuffleBackgroundTextures(SDL_Texture* (*textures)[24]);

void shuffleTextureSet(SDL_Texture* (*textures)[12]);

int paintPauseScreen(PauseScreen* pause_screen, SDL_Renderer* renderer);

/*
 * Event handling
 */

void pauseScreenMousePressed(const PauseScreen* pause_screen, SDL_FPoint mouse_pos);

int pauseScreenMouseReleased(PauseScreen* pause_screen, SDL_FPoint mouse_pos, GameData** game_data);

void pauseScreenMouseDragged(const PauseScreen* pause_screen, SDL_FPoint mouse_pos);

/*
 * Helper functions
 */

static void setPauseScreenBackground(PauseScreen* pause_screen, SDL_Renderer* renderer);

static void setPauseScreenButtons(PauseScreen* pause_screen);

#endif //AFLICKEROFSTILLNESS_PAUSE_SCREEN_H
