#ifndef AFLICKEROFSTILLNESS_PROGRAM_DATA_H
#define AFLICKEROFSTILLNESS_PROGRAM_DATA_H
#include "checkpoints.h"
#include "constants.h"
#include "player.h"
#include "entity.h"

enum {
    GRAPHICS_QUALITY_LOW, GRAPHICS_QUALITY_MEDIUM, GRAPHICS_QUALITY_HIGH
};

typedef struct {
    bool fullscreen;
    uint8_t frame_rate_cap;
    uint8_t sensitivity;
    uint8_t graphics_quality;
}UserData;

typedef struct {
    uint8_t level_index, save_index;
    uint8_t num_map_rows, num_map_cols;
    uint8_t num_checkpoints_hit;
    int map_grid[MAX_MAP_SIZE][MAX_MAP_SIZE];
    int chunk_grid[MAX_MAP_SIZE][MAX_MAP_SIZE];
    uint8_t texture_locations[MAX_MAP_SIZE][MAX_MAP_SIZE][6];
    uint8_t sky_texture_index;
    uint8_t num_entities;
    Entity* entities[MAX_NUM_ENTITIES];
    CheckpointQueue* checkpoint_queue;
    Player* player;
}GameData;

void initGameData(GameData** game_data);

void destroyGameData(GameData** game_data);

void initUserData(UserData** user_data);

void resetUserData(UserData* user_data);

void destroyUserData(UserData** user_data);

#endif //AFLICKEROFSTILLNESS_PROGRAM_DATA_H
