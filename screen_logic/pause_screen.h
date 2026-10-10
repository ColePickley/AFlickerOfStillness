#ifndef AFLICKEROFSTILLNESS_PAUSE_SCREEN_H
#define AFLICKEROFSTILLNESS_PAUSE_SCREEN_H
#include "settings_screen.h"

typedef struct {
    bool paint_pause_screen;
    bool paint_settings_screen;
    bool do_entrance_animation;
    bool do_exit_animation;
    float animation_state;
    uint64_t last_paint_time;
    SettingsScreen* settings_screen;
    float background_texture_width, background_texture_height;
    SDL_Texture* background_textures[15];
    Label* pause_label;
    Button* resume_button;
    Button* last_checkpoint_button;
    Button* restart_level_button;
    Button* settings_button;
    Button* exit_button;
}PauseScreen;

void initPauseScreen(PauseScreen** pause_screen, SDL_Renderer* renderer);

void shuffleImages(PauseScreen* pause_screen);

void paintPauseScreen(PauseScreen* pause_screen, SDL_Renderer* renderer, bool* paint_prev_screen,
    bool* paint_pause_screen);

void pauseScreenMousePressed(const PauseScreen* pause_screen, SDL_FPoint mouse_pos);

void pauseScreenMouseReleased(PauseScreen* pause_screen, SDL_FPoint mouse_pos, bool* program_running,
    uint8_t* current_screen, uint8_t* prev_screen, UserData* user_data, SDL_Window* window);

void pauseScreenMouseDragged(const PauseScreen* pause_screen, SDL_FPoint mouse_pos, UserData* user_data);

void destroyPauseScreen(PauseScreen* pause_screen);

static void setPauseScreenBackground(PauseScreen* pause_screen, SDL_Renderer* renderer);

static void setPauseScreenLabels(PauseScreen* pause_screen);

static void setPauseScreenButtons(PauseScreen* pause_screen);

#endif //AFLICKEROFSTILLNESS_PAUSE_SCREEN_H
