#ifndef AFLICKEROFSTILLNESS_PAUSE_SCREEN_H
#define AFLICKEROFSTILLNESS_PAUSE_SCREEN_H
#include "settings_screen.h"

typedef struct {
    bool paint_pause_screen;
    bool paint_settings_screen;
    SettingsScreen* settings_screen;
    Label* game_label;
    Button* play_button;
    Button* settings_button;
    Button* exit_button;
}PauseScreen;

#endif //AFLICKEROFSTILLNESS_PAUSE_SCREEN_H
