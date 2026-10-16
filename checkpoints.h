#ifndef AFLICKEROFSTILLNESS_CHECKPOINTS_H
#define AFLICKEROFSTILLNESS_CHECKPOINTS_H
#include "screen_logic/label.h"

typedef struct {
    Label* label;
}DialogueBox;

typedef struct CheckpointPage CheckpointPage;

struct CheckpointPage {
    Label* label;
    const DialogueBox* page_dialogue[10];
    CheckpointPage* prev_page;
    CheckpointPage* next_page;
};

typedef struct {
    int x;
    int y;
    CheckpointPage* start_page;
}Checkpoint;

typedef struct CheckpointQueueNode CheckpointQueueNode;

struct CheckpointQueueNode {
    Checkpoint* checkpoint;
    CheckpointQueueNode* next;
};

typedef struct {
    int num_checkpoints;
    CheckpointQueueNode* front;
    CheckpointQueueNode* back;
}CheckpointQueue;

void initDialogueBox(DialogueBox* dialogue_box, SDL_FRect rect, const char* text, int alignment, SDL_Color color);

void destroyDialogueBox(DialogueBox* dialogue_box);

void initCheckpointPage(CheckpointPage** checkpoint_page, DialogueBox* page_dialogue[10], CheckpointPage* prev_page,
    CheckpointPage* next_page);

void destroyCheckpointPage(CheckpointPage* checkpoint_page);

void initCheckpoint(Checkpoint** checkpoint, int x, int y, CheckpointPage* start_page);

void destroyCheckpoint(Checkpoint** checkpoint);

void initCheckpointQueue(CheckpointQueue** checkpoint_queue);

void destroyCheckpointQueue(CheckpointQueue** checkpoint_queue);

void enqueueCheckpoint(CheckpointQueue* queue, Checkpoint* checkpoint);

Checkpoint* dequeueCheckpoint(CheckpointQueue* queue);

Checkpoint* peekCheckpoint(const CheckpointQueue* queue);

bool isEmptyCheckpointQueue(const CheckpointQueue* queue);

#endif //AFLICKEROFSTILLNESS_CHECKPOINTS_H
