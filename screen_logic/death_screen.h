#ifndef AFLICKEROFSTILLNESS_DEATH_SCREEN_H
#define AFLICKEROFSTILLNESS_DEATH_SCREEN_H
#include "button.h"
#include "../program_data.h"

typedef struct {
    bool do_entrance_animation;
    bool do_exit_animation;
    Label* game_over_label;
    Button* last_checkpoint_button;
    Button* restart_level_button;
    Button* settings_button;
    Button* exit_button;
    uint8_t next_screen;
    uint8_t last_paint_time;
    double screen_alpha;
    float animation_speed;
    SDL_Surface* background_surface;
    SDL_Renderer* background_renderer;
    SDL_FPoint last_points_1[4];
    SDL_FPoint last_points_2[4];
}DeathScreen;

void initDeathScreen(DeathScreen** death_screen);

void destroyDeathScreen(DeathScreen** death_screen);

int paintDeathScreen(DeathScreen* death_screen, SDL_Renderer* renderer);

void doPrimaryAnimation(DeathScreen* death_screen);

void doExitAnimation(DeathScreen* death_screen);

/*
 *----------------------------------------------------------------------------------------------------------------------
 * Event handling
 *----------------------------------------------------------------------------------------------------------------------
 */

void deathScreenMousePressed(const DeathScreen* death_screen, SDL_FPoint mouse_pos);

int deathScreenMouseReleased(DeathScreen* death_screen, SDL_FPoint mouse_pos, GameData** game_data);

void deathScreenMouseDragged(const DeathScreen* death_screen, SDL_FPoint mouse_pos);

void deathScreenMouseMoved(const DeathScreen* death_screen, SDL_FPoint mouse_pos);

/*
 *----------------------------------------------------------------------------------------------------------------------
 * Helper functions
 *----------------------------------------------------------------------------------------------------------------------
 */

static void setLayer1NewPoint(SDL_FPoint* new_point, uint8_t pt_index, float direction);

static void setLayer2NewPoint(SDL_FPoint* new_point, uint8_t pt_index, float direction);

static void setDeathScreenLabel(DeathScreen* death_screen);

static void setDeathScreenButtons(DeathScreen* death_screen);

#endif //AFLICKEROFSTILLNESS_DEATH_SCREEN_H
