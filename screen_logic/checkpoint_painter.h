#ifndef AFLICKEROFSTILLNESS_CHECKPOINT_PAINTER_H
#define AFLICKEROFSTILLNESS_CHECKPOINT_PAINTER_H
#include "../checkpoints.h"
#include "../player.h"

extern Label* page_label;
extern Button* back_button;
extern Button* next_button;
extern Button* done_button;

bool initCheckpointPainter();

void destroyCheckpointPainter();

void paintCheckpoint(SDL_Renderer* renderer, const Checkpoint* checkpoint);

/*
 *----------------------------------------------------------------------------------------------------------------------
 * Event handling
 *----------------------------------------------------------------------------------------------------------------------
 */

void checkpointMousePressed(SDL_FPoint mouse_pos);

void checkpointMouseReleased(SDL_FPoint mouse_pos, Checkpoint* checkpoint, Player* player);

void checkpointMouseDragged(SDL_FPoint mouse_pos);

#endif //AFLICKEROFSTILLNESS_CHECKPOINT_PAINTER_H
