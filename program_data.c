#include "program_data.h"

#include <stdio.h>
#include <stdlib.h>

void initGameData(GameData** game_data) {
    *game_data = malloc(sizeof(GameData));
    if (*game_data == NULL) {
        printf("Failed to allocate game data.\n");
    }
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
    CheckpointQueue* checkpoint_queue;
    initCheckpointQueue(&checkpoint_queue);
    (*game_data)->checkpoint_queue = checkpoint_queue;
    (*game_data)->player = NULL;
}

void destroyGameData(GameData** game_data) {
    if (*game_data == NULL) return;
    destroyCheckpointQueue(&(*game_data)->checkpoint_queue);
    if ((*game_data)->player != NULL) {
        destroyPlayer(&(*game_data)->player);
        free((*game_data)->player);
    }
    for (int i = 0; i < (*game_data)->num_entities; i++) {
        destroyEntity(&(*game_data)->entities[i]);
    }
    free(*game_data);
    *game_data = NULL;
}

void initUserData(UserData** user_data) {
    *user_data = malloc(sizeof(UserData));
    (*user_data)->frame_rate_cap = 60;
    (*user_data)->fullscreen = false;
    (*user_data)->graphics_quality = GRAPHICS_QUALITY_LOW;
    (*user_data)->sensitivity = 50;
}

void resetUserData(UserData* user_data) {
    user_data->frame_rate_cap = 60;
    user_data->fullscreen = false;
    user_data->graphics_quality = GRAPHICS_QUALITY_LOW;
    user_data->sensitivity = 50;
}

void destroyUserData(UserData** user_data) {
    if (*user_data == NULL) return;
    free(*user_data);
    *user_data = NULL;
}

void addEntity(GameData* game_data, Entity* entity) {
    game_data->num_entities++;
    game_data->entities[game_data->num_entities-1] = entity;
}

void removeEntity(GameData* game_data, const int entity_index) {
    destroyEntity(&game_data->entities[entity_index]);
    game_data->entities[entity_index] = NULL;
    for (int i = entity_index; i < game_data->num_entities - 1; i++) {
        game_data->entities[i] = game_data->entities[i + 1];
    }
    game_data->num_entities--;
}