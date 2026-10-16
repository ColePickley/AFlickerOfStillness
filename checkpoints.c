#include "checkpoints.h"

#include "constants.h"

void initDialogueBox(DialogueBox* dialogue_box, const SDL_FRect rect, const char* text, const int alignment,
                     const SDL_Color color) {
    const LabelStyleData style_data = {.boarder_thickness = 2, .bevel_radius = 5, .boarder_color = color};
    LabelTextData text_data;
    initLabelTextData(&text_data, text, DIALOGUE_FONT_SIZE, DIALOGUE_LINE_SPACING, 2, 2,
        ALIGN_CENTER, alignment, color);
    const SDL_Color fill_color = {.r = 255, .g = 255, .b = 255, .a = 255};
    initLabel(&dialogue_box->label, rect, fill_color, style_data, text_data);
}

void destroyDialogueBox(DialogueBox* dialogue_box) {
    destroyLabel(dialogue_box->label);
    dialogue_box->label = NULL;
}

void initCheckpointPage(CheckpointPage** checkpoint_page, DialogueBox* page_dialogue[10], CheckpointPage* prev_page,
        CheckpointPage* next_page) {
    *checkpoint_page = malloc(sizeof(CheckpointPage));
    const SDL_FRect page_rect = {.x = (WINDOW_WIDTH - 500.0f) / 2.0f, .y = (WINDOW_HEIGHT - 400.0f) / 2.0f, .w = 500, .h = 400};
    const SDL_Color fill_color = {.r = 255, .g = 255, .b = 255, .a = 255};
    const SDL_Color boarder_color = {.r = 255, .g = 255, .b = 255, .a = 255};
    const LabelStyleData style_data = {.boarder_thickness = 2.0f, .bevel_radius = 5.0f, .boarder_color = boarder_color};
    LabelTextData text_data;
    initLabelTextData(&text_data, NULL, 0, 0, 0, 0, 0, 0, fill_color);
    Label* page_label;
    initLabel(&page_label, page_rect, fill_color, style_data, text_data);
    (*checkpoint_page)->label = page_label;
    memcpy((*checkpoint_page)->page_dialogue, page_dialogue, sizeof(DialogueBox*) * 10);
    (*checkpoint_page)->prev_page = prev_page;
    (*checkpoint_page)->next_page = next_page;
}

void destroyCheckpointPage(CheckpointPage* checkpoint_page) {
    destroyLabel(checkpoint_page->label);
    free(checkpoint_page);
    checkpoint_page = NULL;
}

void initCheckpoint(Checkpoint** checkpoint, const int x, const int y, CheckpointPage* start_page) {
    *checkpoint = malloc(sizeof(Checkpoint));
    (*checkpoint)->x = x;
    (*checkpoint)->y = y;
    (*checkpoint)->start_page = start_page;
}

void destroyCheckpoint(Checkpoint** checkpoint) {
    CheckpointPage* current_page = (*checkpoint)->start_page;
    while (current_page != NULL) {
        CheckpointPage* next_page = current_page->next_page;
        destroyCheckpointPage(current_page);
        current_page = next_page;
    }
    free(*checkpoint);
    *checkpoint = NULL;
}

void initCheckpointQueue(CheckpointQueue** checkpoint_queue) {
    *checkpoint_queue = malloc(sizeof(CheckpointQueue));
    (*checkpoint_queue)->front = NULL;
    (*checkpoint_queue)->back = NULL;
    (*checkpoint_queue)->num_checkpoints = 0;
}

void destroyCheckpointQueue(CheckpointQueue** checkpoint_queue) {
    while (!isEmptyCheckpointQueue(*checkpoint_queue)) {
        Checkpoint* checkpoint = dequeueCheckpoint(*checkpoint_queue);
        destroyCheckpoint(&checkpoint);
    }
    free(*checkpoint_queue);
    *checkpoint_queue = NULL;
}

void enqueueCheckpoint(CheckpointQueue* queue, Checkpoint* checkpoint) {
    CheckpointQueueNode* new_node = malloc(sizeof(CheckpointQueueNode));
    new_node->checkpoint = checkpoint;
    new_node->next = NULL;
    if (queue->back == NULL) {
        queue->front = new_node;
        queue->back = new_node;
        return;
    }
    queue->back->next = new_node;
    queue->back = new_node;
    queue->num_checkpoints++;
}

Checkpoint* dequeueCheckpoint(CheckpointQueue* queue) {
    if (isEmptyCheckpointQueue(queue)) {
        return NULL;
    }
    CheckpointQueueNode* temp_node = queue->front;
    Checkpoint* front_checkpoint = temp_node->checkpoint;
    queue->front = queue->front->next;
    if (queue->front == NULL) {
        queue->back = NULL;
    }
    free(temp_node);
    queue->num_checkpoints--;
    return front_checkpoint;
}

Checkpoint* peekCheckpoint(const CheckpointQueue* queue) {
    if (isEmptyCheckpointQueue(queue)) {
        return NULL;
    }
    return queue->front->checkpoint;
}

bool isEmptyCheckpointQueue(const CheckpointQueue* queue) {
    return queue->front == NULL;
}