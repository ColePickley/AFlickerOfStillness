#ifndef AFLICKEROFSTILLNESS_CHECKPOINTS_H
#define AFLICKEROFSTILLNESS_CHECKPOINTS_H
#include "screen_logic/button.h"
#include "screen_logic/label.h"

#define CHECKPOINT_PAGE_WIDTH 1000
#define CHECKPOINT_PAGE_HEIGHT 600
#define CHECKPOINT_PAGE_X ((WINDOW_WIDTH - CHECKPOINT_PAGE_WIDTH) / 2.0f)
#define CHECKPOINT_PAGE_Y ((WINDOW_HEIGHT - CHECKPOINT_PAGE_HEIGHT) / 2.0f)
#define DIALOGUE_BOX_WIDTH 500
#define DIALOGUE_BOX_SPACING 10
#define DIALOGUE_FONT_SIZE 20
#define DIALOGUE_LINE_SPACING 5
#define MAX_NUM_PAGE_LABELS 10

typedef struct CheckpointPage CheckpointPage;

struct CheckpointPage {
    const Label* page_labels[10];
    CheckpointPage* prev_page;
    CheckpointPage* next_page;
};

typedef struct {
    int x;
    int y;
    CheckpointPage* current_page;
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

void initPageLabel(Label** label, SDL_FRect rect, const char* text, int alignment, SDL_Color color);

void initCheckpointPage(CheckpointPage** checkpoint_page, Label* page_labels[10], CheckpointPage* prev_page,
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
