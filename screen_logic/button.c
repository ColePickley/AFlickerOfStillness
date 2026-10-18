#include "button.h"

#include <stdlib.h>
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

bool isPointWithinButton(const Button* button, const SDL_FPoint mouse_pos) {
    return SDL_PointInRectFloat(&mouse_pos, &button->label->rect);
}


bool buttonPressCheck(Button* button, const SDL_FPoint mouse_pos) {
    if (isPointWithinButton(button, mouse_pos)) {
        turnOnButton(button);
        button->is_selected = true;
        return true;
    }
    return false;
}

int buttonGroupPressCheck(const uint8_t num_buttons, Button* const buttons[num_buttons], const SDL_FPoint mouse_pos) {
    int clicked_button_index = -1;
    for (int i = 0; i < num_buttons; i++) {
        if (buttonPressCheck(buttons[i], mouse_pos)) {
            clicked_button_index = i;
            break;
        }
    }
    if (clicked_button_index != -1) {
        for (int i = 0; i < num_buttons; i++) {
            if (i == clicked_button_index) continue;
            turnOffButton(buttons[i]);
            buttons[i]->is_selected = false;
        }
    }
    return clicked_button_index;
}

bool buttonReleaseCheck(Button* button, const SDL_FPoint mouse_pos) {
    turnOffButton(button);
    if (isPointWithinButton(button, mouse_pos) && button->is_selected) {
        button->is_selected = false;
        return true;
    }
    button->is_selected = false;
    return false;
}

bool buttonMouseDragCheck(Button* button, const SDL_FPoint mouse_pos) {
    if (isPointWithinButton(button, mouse_pos) && button->is_selected) {
        turnOnButton(button);
        return true;
    }
    turnOffButton(button);
    return false;
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