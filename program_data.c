#include "program_data.h"

#include "constants.h"
#include "screen_logic/label.h"
#include <stdlib.h>

void initGameData(GameData** game_data) {
    *game_data = malloc(sizeof(GameData));
    (*game_data)->level_index = 0;
    (*game_data)->save_index = 0;
    (*game_data)->num_checkpoints_hit = 0;
    (*game_data)->num_map_rows = 0;
    (*game_data)->num_map_cols = 0;
    for (int i = 0; i < MAX_MAP_SIZE; i++) {
        for (int j = 0; j < MAX_MAP_SIZE; j++) {
            (*game_data)->map_grid[i][j] = 0;
            (*game_data)->chunk_grid[i][j] = 0;
            for (int k = 0; k < 6; k++) {
                (*game_data)->texture_locations[i][j][k] = 0;
            }
        }
    }
    (*game_data)->num_entities = 0;
    for (int i = 0; i < MAX_NUM_ENTITIES; i++) {
        (*game_data)->entities[i] = NULL;
    }
    CheckpointQueue* checkpoint_stack;
    initCheckpointQueue(&checkpoint_stack);
    (*game_data)->checkpoint_queue = checkpoint_stack;
    (*game_data)->player = NULL;
}

void destroyGameData(GameData* game_data) {
    free(game_data);
    game_data = NULL;
}

void initUserData(UserData** user_data) {
    *user_data = malloc(sizeof(UserData));
    (*user_data)->frame_rate_cap = 30;
    (*user_data)->fullscreen = false;
    (*user_data)->graphics_quality = GRAPHICS_QUALITY_LOW;
    (*user_data)->sensitivity = 50;
}

void resetUserData(UserData* user_data) {
    user_data->frame_rate_cap = 30;
    user_data->fullscreen = false;
    user_data->graphics_quality = GRAPHICS_QUALITY_LOW;
    user_data->sensitivity = 50;
}

void destroyUserData(UserData* user_data) {
    free(user_data);
    user_data = NULL;
}