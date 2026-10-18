#ifndef AFLICKEROFSTILLNESS_SAVE_HANDLING_H
#define AFLICKEROFSTILLNESS_SAVE_HANDLING_H
#include "../program_data.h"

enum {
    FILE_SECTION_SAVE_MAP_GRID, FILE_SECTION_NUM_CHECKPOINTS_HIT, FILE_SECTION_SAVE_ENTITIES, FILE_SECTION_SAVE_PLAYER
};

bool loadSave(GameData* game_data);

int getLevelIndex(uint8_t save_index);

void setSaveText(char* save_text, size_t save_text_size, uint8_t save_index);

static void setSaveMapGrid(GameData* game_data, char line[500], int line_num, int* file_section);

static void dequeHitCheckpoints(const GameData* game_data, char line[500], int* file_section);

static void setSaveEntities(GameData* game_data, char line[500], int* file_section);

static void setSavePlayer(const GameData* game_data, char line[500], int* file_section);

void writeSave(const GameData* game_data);

static void setNewSaveText(char* save_text, size_t save_text_size);

void deleteSave(uint8_t save_index);

#endif //AFLICKEROFSTILLNESS_SAVE_HANDLING_H
