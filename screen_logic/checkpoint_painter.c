#include "checkpoint_painter.h"

#include "../constants.h"
#include "button.h"

Label* page_label;
Button* back_button;
Button* next_button;
Button* done_button;

bool initCheckpointPainter() {
    const SDL_FRect page_rect = {
        .x = CHECKPOINT_PAGE_X,
        .y = CHECKPOINT_PAGE_Y,
        .w = CHECKPOINT_PAGE_WIDTH,
        .h = CHECKPOINT_PAGE_HEIGHT
    };
    const LabelStyleData page_style_data = {
        .border_thickness = 2.0f,
        .bevel_radius = 10.0f,
        .border_color = (SDL_Color) {.r = 255, .g = 255, .b = 255, .a = 255}
    };
    LabelTextData page_text_data;
    initDefaultLabelTextData(&page_text_data);
    initLabel(&page_label, page_rect, (SDL_Color) {.r = 0, .g = 0, .b = 0, .a = 100}, page_style_data, page_text_data);

    const LabelStyleData style_data = {
        .bevel_radius = 10.0f,
        .border_thickness = 2.0f,
        .border_color = (SDL_Color) {.r = 255, .g = 255, .b = 255, .a = 255}
    };
    const ButtonColorData color_data = {
        .boarder_click_color = (SDL_Color) {.r = 150, .g = 0, .b = 0, .a = 255},
        .boarder_idle_color = (SDL_Color) {.r = 255, .g = 255, .b = 255, .a = 255},
        .fill_click_color = (SDL_Color) {.r = 0, .g = 0, .b = 0, .a = 100},
        .fill_idle_color = (SDL_Color) {.r = 0, .g = 0, .b = 0, .a = 100},
        .text_click_color = (SDL_Color) {.r = 150, .g = 0, .b = 0, .a = 255},
        .text_idle_color = (SDL_Color) {.r = 255, .g = 255, .b = 255, .a = 255}
    };

    const SDL_FRect back_rect = {
        .x = CHECKPOINT_PAGE_X + 20.0f,
        .y = CHECKPOINT_PAGE_Y + CHECKPOINT_PAGE_HEIGHT - 70.0f,
        .w = 150.0f,
        .h = 50.0f
    };
    LabelTextData back_text_data;
    initLabelTextData(&back_text_data, "Back", FONT_PIXELOID, 30.0f, 0.0f,
        0.0f,0.0f, ALIGN_CENTER, ALIGN_CENTER,
        (SDL_Color) {.r = 255, .g = 255, .b = 255, .a = 255});
    Label* back_label;
    initLabel(&back_label, back_rect, (SDL_Color) {.r = 0, .g = 0, .b = 0, .a = 100}, style_data, back_text_data);
    initButton(&back_button, back_label, color_data);

    const SDL_FRect next_rect = {
        .x = CHECKPOINT_PAGE_X + CHECKPOINT_PAGE_WIDTH - 170.0f,
        .y = CHECKPOINT_PAGE_Y + CHECKPOINT_PAGE_HEIGHT - 70.0f,
        .w = 150.0f,
        .h = 50.0f
    };
    LabelTextData next_text_data;
    initLabelTextData(&next_text_data, "Next", FONT_PIXELOID, 30.0f, 0.0f,
        0.0f,0.0f, ALIGN_CENTER, ALIGN_CENTER,
        (SDL_Color) {.r = 255, .g = 255, .b = 255, .a = 255});
    Label* next_label;
    initLabel(&next_label, next_rect, (SDL_Color) {.r = 0, .g = 0, .b = 0, .a = 100}, style_data, next_text_data);
    initButton(&next_button, next_label, color_data);

    const SDL_FRect done_rect = {
        .x = CHECKPOINT_PAGE_X + CHECKPOINT_PAGE_WIDTH - 170.0f,
        .y = CHECKPOINT_PAGE_Y + CHECKPOINT_PAGE_HEIGHT - 70.0f,
        .w = 150.0f,
        .h = 50.0f
    };
    LabelTextData done_text_data;
    initLabelTextData(&done_text_data, "Done", FONT_PIXELOID, 30.0f, 0.0f,
        0.0f,0.0f, ALIGN_CENTER, ALIGN_CENTER,
        (SDL_Color) {.r = 255, .g = 255, .b = 255, .a = 255});
    Label* done_label;
    initLabel(&done_label, done_rect, (SDL_Color) {.r = 0, .g = 0, .b = 0, .a = 100}, style_data, done_text_data);
    initButton(&done_button, done_label, color_data);
    return true;
}

void destroyCheckpointPainter() {
    destroyLabel(&page_label);
    destroyButton(&back_button);
    destroyButton(&next_button);
    destroyButton(&done_button);
}

void paintCheckpoint(SDL_Renderer* renderer, const Checkpoint* checkpoint) {
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 150);
    SDL_RenderFillRect(renderer, &(SDL_FRect) {.x = 0, .y = 0, .w = WINDOW_WIDTH, .h = WINDOW_HEIGHT});
    paintLabel(renderer, page_label);
    for (int i = 0; i < MAX_NUM_PAGE_LABELS; i++) {
        if (checkpoint->current_page->page_labels[i] != NULL) {
            paintLabel(renderer, checkpoint->current_page->page_labels[i]);
        } else {
            break;
        }
    }
    if (checkpoint->current_page->prev_page != NULL) {
        paintLabel(renderer, back_button->label);
    }
    if (checkpoint->current_page->next_page != NULL) {
        paintLabel(renderer, next_button->label);
    } else {
        paintLabel(renderer, done_button->label);
    }
}

/*
 *----------------------------------------------------------------------------------------------------------------------
 * Event handling
 *----------------------------------------------------------------------------------------------------------------------
 */

void checkpointMousePressed(const SDL_FPoint mouse_pos) {
    if (buttonPressCheck(back_button, mouse_pos)) return;
    buttonPressCheck(next_button, mouse_pos);
    buttonPressCheck(done_button, mouse_pos);
}

void checkpointMouseReleased(const SDL_FPoint mouse_pos, Checkpoint* checkpoint, Player* player) {
    if (buttonReleaseCheck(back_button, mouse_pos) && checkpoint->current_page->prev_page != NULL) {
        checkpoint->current_page = checkpoint->current_page->prev_page;
    }
    if (buttonReleaseCheck(next_button, mouse_pos) && checkpoint->current_page->next_page != NULL) {
        checkpoint->current_page = checkpoint->current_page->next_page;
        turnOffButton(done_button);
    } else if (buttonReleaseCheck(done_button, mouse_pos) && checkpoint->current_page->next_page == NULL) {
        player->checkpoint_active = false;
        player->force_checkpoint_inactive = true;
    }
}

void checkpointMouseDragged(const SDL_FPoint mouse_pos) {
    buttonMouseDragCheck(back_button, mouse_pos);
    buttonMouseDragCheck(next_button, mouse_pos);
    buttonMouseDragCheck(done_button, mouse_pos);
}