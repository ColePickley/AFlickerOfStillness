#include "save_handling.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

bool loadSave(GameData* game_data) {
    const char* base_path = SDL_GetBasePath();
    if (!base_path) {
        SDL_Log("Could not determine base path: %s", SDL_GetError());
        return false;
    }
    char file_path[21] = "user_data\\saves\\save";
    char file_type[5] = ".csv";
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s%s%d%s", base_path, file_path, game_data->save_index, file_type);
    FILE* file = fopen(full_path, "r");
    if (file == NULL) {
        printf("Path: %s | ", full_path);
        perror("Error loading save file");
        return false;
    }

    // clear variables that will be reset
    for (int i = 0; i < MAX_MAP_SIZE; i++) {
        for (int j = 0; j < MAX_MAP_SIZE; j++) {
            game_data->map_grid[i][j] = 0;
        }
    }
    game_data->num_entities = 0;
    for (int i = 0; i < MAX_NUM_ENTITIES; i++) {
        destroyEntity(game_data->entities[i]);
        game_data->entities[i] = NULL;
    }

    char line[500];
    int file_section = 0;
    int line_num = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line_num < 4) {
            line_num++;
            continue;
        }
        switch (file_section) {
            case FILE_SECTION_SAVE_MAP_GRID:
                setSaveMapGrid(game_data, line, line_num, &file_section);
                break;
            case FILE_SECTION_NUM_CHECKPOINTS_HIT:
                dequeHitCheckpoints(game_data, line, &file_section);
                break;
            case FILE_SECTION_SAVE_ENTITIES:
                setSaveEntities(game_data, line, &file_section);
                break;
            case FILE_SECTION_SAVE_PLAYER:
                setSavePlayer(game_data, line, &file_section);
                break;
            default:
                break;
        }
        line_num++;
    }
    fclose(file);
    return true;
}

int getLevelIndex(const uint8_t save_index) {
    const char* base_path = SDL_GetBasePath();
    if (!base_path) {
        SDL_Log("Could not determine base path: %s", SDL_GetError());
        return 0;
    }
    char file_path[21] = "user_data\\saves\\save";
    char file_type[5] = ".csv";
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s%s%d%s", base_path, file_path, save_index, file_type);
    FILE* file = fopen(full_path, "r");
    if (file == NULL) {
        printf("Path: %s | ", full_path);
        perror("Error opening save file");
        return 0;
    }
    char line[500];
    if (fgets(line, sizeof(line), file) == NULL) {
        return 0;
    }
    line[strcspn(line, "\r\n")] = '\0';
    const char* token = strtok(line, ",");
    char* end_token;
    return (int) strtol(token, &end_token, 10);
    fclose(file);
}

void setSaveText(char* save_text, const size_t save_text_size, const uint8_t save_index) {
    const char* base_path = SDL_GetBasePath();
    if (!base_path) {
        SDL_Log("Could not determine base path: %s", SDL_GetError());
        return;
    }
    char file_path[21] = "user_data\\saves\\save";
    char file_type[5] = ".csv";
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s%s%d%s", base_path, file_path, save_index, file_type);
    FILE* file = fopen(full_path, "r");
    if (file == NULL) {
        const char* new_save_text = "EMPTY";
        snprintf(save_text, save_text_size, "%s", new_save_text);
        return;
    }
    char line[500];
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    if (fgets(line, sizeof(line), file) == NULL) {
        return;
    }
    line[strcspn(line, "\r\n")] = '\0';
    const char* token = strtok(line, ",");
    snprintf(save_text, save_text_size, "%s", token);
    fclose(file);
}

static void setSaveMapGrid(GameData* game_data, char line[500], const int line_num, int* file_section) {
    const char* token = strtok(line, ",");
    if (token[0] == '"') {
        *file_section += 1;
        return;
    }
    int token_num = 0;
    while (token != NULL) {
        char* end_token;
        const int token_as_int = (int) strtol(token, &end_token, 10);
        game_data->map_grid[line_num - 4][token_num] = token_as_int;
        token = strtok(NULL, ",");
        token_num += 1;
    }
}

static void dequeHitCheckpoints(const GameData* game_data, char line[500], int* file_section) {
    const char* token = strtok(line, ",");
    if (token[0] == '"') {
        *file_section += 1;
        return;
    }
    char* end_token;
    const int token_as_int = (int) strtol(token, &end_token, 10);
    for (int i = 0; i < token_as_int; i++) {
        dequeueCheckpoint(game_data->checkpoint_queue);
    }
}

static void setSaveEntities(GameData* game_data, char line[500], int* file_section) {
    const char *token = strtok(line, ",");
    if (token[0] == '"') {
        *file_section += 1;
        return;
    }
    int token_num = 0;
    double x = -1;
    double y = -1;
    int type_index = -1;
    int health = -1;
    int melee_cool_down_state = -1;
    int projectile_cool_down_state = -1;
    int ammo_count = -1;
    int mag_count = -1;
    while (token != NULL && token_num < 8) {
        char* end_token;
        const int token_as_int = (int) strtol(token, &end_token, 10);
        const double token_as_double = strtod(token, &end_token);
        switch (token_num) {
            case 0:
                x = token_as_double;
                break;
            case 1:
                y = token_as_double;
                break;
            case 2:
                type_index = token_as_int;
                break;
            case 3:
                health = token_as_int;
                break;
            case 4:
                melee_cool_down_state = token_as_int;
                break;
            case 5:
                projectile_cool_down_state = token_as_int;
                break;
            case 6:
                ammo_count = token_as_int;
                break;
            case 7:
                mag_count = token_as_int;
                break;
            default:
                break;
        }
        token = strtok(NULL, ",");
        token_num++;
    }
    Entity* entity;
    initDefaultEntity(&entity, type_index, x, y);
    game_data->entities[game_data->num_entities] = entity;
    game_data->entities[game_data->num_entities]->health = health;
    game_data->entities[game_data->num_entities]->melee_cool_down = melee_cool_down_state;
    game_data->entities[game_data->num_entities]->projectile_cool_down = projectile_cool_down_state;
    game_data->entities[game_data->num_entities]->ammo_count = ammo_count;
    game_data->entities[game_data->num_entities]->mag_count = mag_count;
    game_data->num_entities++;
}

static void setSavePlayer(GameData* game_data, char line[500], int* file_section) {
    destroyPlayer(&game_data->player);
    initPlayer(&game_data->player);
    const char* token = strtok(line, ",");
    if (token[0] == '"') {
        *file_section += 1;
        return;
    }
    int token_num = 0;
    while (token != NULL && token_num < 8) {
        char* end_token;
        const double token_as_double = strtod(token, &end_token);
        const int token_as_int = (int) strtol(token, &end_token, 10);
        switch (token_num) {
            case 0:
                game_data->player->x = token_as_double;
                break;
            case 1:
                game_data->player->y = token_as_double;
                break;
            case 2:
                game_data->player->rotation = token_as_double;
                break;
            case 3:
                game_data->player->health = token_as_int;
                break;
            case 4:
                game_data->player->primary_weapon_index = token_as_int;
                break;
            case 5:
                game_data->player->ammo_count = token_as_int;
                break;
            case 6:
                game_data->player->mag_count = token_as_int;
                break;
            case 7:
                game_data->player->secondary_weapon_index = token_as_int;
                break;
            default:
                break;
        }
        token = strtok(NULL, ",");
        token_num++;
    }
}

void writeSave(const GameData* game_data) {
    const char* base_path = SDL_GetBasePath();
    if (!base_path) {
        SDL_Log("Could not determine base path: %s", SDL_GetError());
        return;
    }
    char file_path[21] = "user_data\\saves\\save";
    char file_type[5] = ".csv";
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s%s%d%s", base_path, file_path, game_data->save_index, file_type);
    FILE* file = fopen(full_path, "w");
    if (file == NULL) {
        perror("Error writing to save file");
        return;
    }
    fprintf(file, "%d\n\"\"\n", game_data->save_index);
    char save_text[100];
    setNewSaveText(save_text, sizeof(save_text));
    fprintf(file, "%s\n\"\"\n", save_text);
    for (int row = 0; row < game_data->num_map_rows; row++) {
        for (int col = 0; col < game_data->num_map_cols; col++) {
            fprintf(file, "%d", game_data->map_grid[row][col]);
            if (col < game_data->num_map_cols - 1) {
                fprintf(file, ",");
            }
        }
        fprintf(file, "\n");
    }
    fprintf(file, "\"\"\n%d\n\"\"\n", game_data->num_checkpoints_hit);
    for (int i = 0; i < game_data->num_entities; i++) {
        const Entity* entity = game_data->entities[i];
        fprintf(file, "%lf,%lf,%d,%d,%d,%d,%d,%d\n", entity->x, entity->y, entity->type_index, entity->health,
            entity->melee_cool_down, entity->projectile_cool_down, entity->ammo_count, entity->mag_count);
    }
    fprintf(file, "\"\"\n%lf,%lf,%lf,%d,%d,%d,%d,%d", game_data->player->x, game_data->player->y,
        game_data->player->rotation, game_data->player->health, game_data->player->primary_weapon_index,
        game_data->player->ammo_count, game_data->player->mag_count, game_data->player->secondary_weapon_index);
    fclose(file);
}

static void setNewSaveText(char* save_text, const size_t save_text_size) {
    const time_t raw_time = time(NULL);
    const struct tm* lt = localtime(&raw_time);
    const int year  = lt->tm_year + 1900;
    const int month = lt->tm_mon + 1;
    const int day   = lt->tm_mday;
    const int hour  = lt->tm_hour;
    const int min   = lt->tm_min;
    snprintf(save_text, save_text_size, "%04d-%02d-%02d %02d:%02d",
        year, month, day, hour, min);
}

void deleteSave(const uint8_t save_index) {
    const char* base_path = SDL_GetBasePath();
    if (!base_path) {
        SDL_Log("Could not determine base path: %s", SDL_GetError());
        return;
    }
    char file_path[21] = "user_data\\saves\\save";
    char file_type[5] = ".csv";
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s%s%d%s", base_path, file_path, save_index, file_type);
    if (remove(full_path) == 0) {
        printf("Save %d cleared successfully.\n", save_index);
    } else {
        printf("Path: %s | ", full_path);
        printf("Error clearing save %d.\n", save_index);
    }
}