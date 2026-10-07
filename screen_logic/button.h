#ifndef AFLICKEROFSTILLNESS_BUTTON_H
#define AFLICKEROFSTILLNESS_BUTTON_H
#include "label.h"
#include "painter.h"

typedef struct {
    SDL_Color fill_click_color;
    SDL_Color fill_idle_color;
    SDL_Color boarder_click_color;
    SDL_Color boarder_idle_color;
    SDL_Color text_click_color;
    SDL_Color text_idle_color;
}ButtonColorData;

typedef struct {
    Label* label;
    bool is_on;
    bool is_selected;
    ButtonColorData color_data;
}Button;

void initButton(Button** button, Label* label, ButtonColorData color_data);

void destroyButton(Button* button);

bool isButtonPressed(const Button* button, SDL_FPoint mouse_pos);

void turnOnButton(Button* button);

void turnOffButton(Button* button);

#endif //AFLICKEROFSTILLNESS_BUTTON_H
