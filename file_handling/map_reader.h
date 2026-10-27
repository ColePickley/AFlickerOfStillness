#ifndef AFLICKEROFSTILLNESS_MAP_READER_H
#define AFLICKEROFSTILLNESS_MAP_READER_H
#include "../program_data.h"

enum {
    FILE_SECTION_MAP_GRID, FILE_SECTION_CHUNK_GRID, FILE_SECTION_TEXTURE_LOCATIONS, FILE_SECTION_CHECKPOINTS,
    FILE_SECTION_ENTITIES, FILE_SECTION_SKY_TEXTURE, FILE_SECTION_PLAYER
};

void readMap(GameData* game_data);

static void setMapGrid(GameData* game_data, char line[500], int line_num, int* file_section);

static void setChunkGrid(GameData* game_data, char line[500], int line_num, int* file_section);

static void setTextureLocations(GameData* game_data, char line[500], int* file_section);

static void setCheckpoint(const GameData* game_data, char line[500], int* file_section);

static void setCheckpointPage(int* page_num, int* dialogue_num, int* alignment, int token_num, int token_as_int,
    char* token, char** text, SDL_Color* color, DialogueBox* (*page_dialogue)[10], CheckpointPage** start_page,
    CheckpointPage** current_page);

static int getAlignment(const char* alignment);

static SDL_FRect getDialogueBoxRect(DialogueBox* box, DialogueBox* page_dialogue[10], int dialogue_num, const char* text, int alignment);

static void setEntities(GameData* game_data, char line[500], int* file_section);

static void setSkyTextureIndex(GameData* game_data, char line[500], int* file_section);

static void setPlayer(GameData* game_data, char line[500], int* file_section);

#endif //AFLICKEROFSTILLNESS_MAP_READER_H
