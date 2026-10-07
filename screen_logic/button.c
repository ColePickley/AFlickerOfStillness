#include "button.h"
#include "label.h"

void initButton(Button** button, Label* label, const ButtonColorData color_data) {
    *button = malloc(sizeof(Button));
    (*button)->label = label;
    (*button)->color_data = color_data;
    (*button)->is_on = false;
    (*button)->is_selected = false;
}

void destroyButton(Button* button) {
    destroyLabel(button->label);
    button->label = NULL;
    if (button != NULL) {
        free(button);
        button = NULL;
    }
}

bool isButtonPressed(const Button* button, const SDL_FPoint mouse_pos) {
    return SDL_PointInRectFloat(&mouse_pos, &button->label->rect);
}

void turnOnButton(Button* button) {
    button->is_on = true;
    button->label->fill_color = button->color_data.fill_click_color;
    button->label->style_data.boarder_color = button->color_data.boarder_click_color;
    button->label->text_data.text_color = button->color_data.text_click_color;
}

void turnOffButton(Button* button) {
    button->is_on = false;
    button->label->fill_color = button->color_data.fill_idle_color;
    button->label->style_data.boarder_color = button->color_data.boarder_idle_color;
    button->label->text_data.text_color = button->color_data.text_idle_color;
}