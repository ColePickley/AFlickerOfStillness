#include "checkpoints.h"

#include <stdio.h>
#include "constants.h"
#include <stdlib.h>
#include "screen_logic/painter.h"

void initPageLabel(Label** label, const SDL_FRect rect, const char* text, const int alignment,
        const SDL_Color color) {
    const LabelStyleData style_data = {.border_thickness = 2, .bevel_radius = 5, .border_color = color};
    LabelTextData text_data;
    initLabelTextData(&text_data, text, FONT_PIXELOID, DIALOGUE_FONT_SIZE,
        DIALOGUE_LINE_SPACING, 2, 5,ALIGN_CENTER,
        alignment, color);
    const SDL_Color fill_color = {.r = 0, .g = 0, .b = 0, .a = 100};
    initLabel(label, rect, fill_color, style_data, text_data);
}

void initCheckpointPage(CheckpointPage** checkpoint_page, Label* page_labels[10], CheckpointPage* prev_page,
        CheckpointPage* next_page) {
    *checkpoint_page = malloc(sizeof(CheckpointPage));
    memcpy((*checkpoint_page)->page_labels, page_labels, sizeof(Label*) * MAX_NUM_PAGE_LABELS);
    (*checkpoint_page)->prev_page = prev_page;
    (*checkpoint_page)->next_page = next_page;
}

void destroyCheckpointPage(CheckpointPage* checkpoint_page) {
    free(checkpoint_page);
    checkpoint_page = NULL;
}

void initCheckpoint(Checkpoint** checkpoint, const int x, const int y, CheckpointPage* start_page) {
    *checkpoint = malloc(sizeof(Checkpoint));
    (*checkpoint)->x = x;
    (*checkpoint)->y = y;
    (*checkpoint)->current_page = start_page;
}

void destroyCheckpoint(Checkpoint** checkpoint) {
    CheckpointPage* current_page = (*checkpoint)->current_page;
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
    if (queue->num_checkpoints == MAX_NUM_CHECKPOINTS) {
        printf("Could not enqueue Checkpoint. Queue is full.\n");
        return;
    }
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
        printf("Could not dequeue Checkpoint. Queue is empty.\n");
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