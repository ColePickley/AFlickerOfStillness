#include "map_reader.h"

#include <stdio.h>
#include <stdlib.h>
#include "../program_data.h"

void readMap(GameData* game_data) {
    const char* base_path = SDL_GetBasePath();
    if (!base_path) {
        SDL_Log("Could not determine base path: %s", SDL_GetError());
        return;
    }
    char target_dir[512];
    SDL_snprintf(target_dir, sizeof(target_dir), "%sgame_data/levels/", base_path);
    int num_files = 0;
    char** files = SDL_GlobDirectory(target_dir, NULL, SDL_GLOB_CASEINSENSITIVE, &num_files);
    if (files == NULL) {
        SDL_Log("Failed to list directory: %s", SDL_GetError());
        return;
    }
    char file_path[24] = "../game_data/levels/";
    char full_path[100];
    snprintf(full_path, sizeof(full_path), "%s%s", file_path, files[game_data->level_index]);
    FILE *file = fopen(full_path, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    char line[500];
    int file_section = 0;
    int line_num = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        switch (file_section) {
            case FILE_SECTION_MAP_GRID:
                setMapGrid(game_data, line, line_num, &file_section);
                break;
            case FILE_SECTION_CHUNK_GRID:
                setChunkGrid(game_data, line, line_num, &file_section);
                break;
            case FILE_SECTION_TEXTURE_LOCATIONS:
                setTextureLocations(game_data, line, &file_section);
                break;
            case FILE_SECTION_CHECKPOINTS:
                setCheckpoint(game_data, line, &file_section);
                break;
            case FILE_SECTION_ENTITIES:
                setEntities(game_data, line, &file_section);
                break;
            case FILE_SECTION_SKY_TEXTURE:
                setSkyTexture(game_data, line, &file_section);
                break;
            case FILE_SECTION_PLAYER:
                setPlayer(game_data, line, &file_section);
                break;
            default:
                break;
        }
        line_num += 1;
    }
    fclose(file);
}

static void setMapGrid(GameData* game_data, char line[500], const int line_num, int* file_section) {
    const char *token = strtok(line, ",");
    if (token[0] == '"') {
        *file_section += 1;
        return;
    }
    game_data->num_map_rows += 1;
    bool do_num_cols_count = false;
    if (game_data->num_map_cols == 0) {
        do_num_cols_count = true;
    }
    int token_num = 0;
    while (token != NULL) {
        char* end_token;
        const int token_as_int = strtol(token, &end_token, 10);
        if (do_num_cols_count) {
            game_data->num_map_cols += 1;
        }
        game_data->map_grid[line_num][token_num] = token_as_int;
        token = strtok(NULL, ",");
        token_num += 1;
    }
}

static void setChunkGrid(GameData* game_data, char line[500], const int line_num, int* file_section) {
    const char *token = strtok(line, ",");
    if (token[0] == '"') {
        *file_section += 1;
        return;
    }
    int token_num = 0;
    const int adjusted_line_num = line_num - game_data->num_map_rows - *file_section;
    while (token != NULL) {
        char* end_token;
        const int token_as_int = strtol(token, &end_token, 10);
        game_data->chunk_grid[adjusted_line_num][token_num] = token_as_int;
        token = strtok(NULL, ",");
        token_num += 1;
    }
}

static void setTextureLocations(GameData* game_data, char line[500], int* file_section) {
    const char *token = strtok(line, ",");
    if (token[0] == '"') {
        *file_section += 1;
        return;
    }
    int token_num = 0;
    int map_row = -1;
    int map_col = -1;
    int side = -1;
    while (token != NULL) {
        char* end_token;
        const int token_as_int = strtol(token, &end_token, 10);
        switch (token_num) {
            case 0:
                map_row = token_as_int;
                break;
            case 1:
                map_col = token_as_int;
                break;
            case 2:
                side = token_as_int;
                break;
            case 3:
                game_data->texture_locations[map_row][map_col][side] = token_as_int;
                token_num = -1;
                break;
            default:
                break;
        }
        token = strtok(NULL, ",");
        token_num += 1;
    }
}

static void setCheckpoint(const GameData* game_data, char line[500], int* file_section) {
    char *token = strtok(line, ",");
    if (token[0] == '"') {
        *file_section += 1;
        return;
    }
    int token_num = 0;
    Checkpoint* checkpoint;
    int x = -1;
    int y = -1;
    int num_pages = 0;
    CheckpointPage* start_page = NULL;
    CheckpointPage* current_page = NULL;
    DialogueBox* page_dialogue[10];
    for (int i = 0; i < 10; i++) {
        page_dialogue[i] = NULL;
    }

    int page_num = -1;
    int dialogue_num = 0;
    char* text = NULL;
    int alignment = -1;
    SDL_Color color = {.r = 0, .g = 0, .b = 0, .a = 255};
    while (token != NULL && page_num < num_pages) {
        char* end_token;
        const int token_as_int = strtol(token, &end_token, 10);
        switch (token_num) {
            case 0:
                x = token_as_int;
                break;
            case 1:
                y = token_as_int;
                break;
            case 2:
                num_pages = token_as_int;
                break;
            default:
                setCheckpointPage(&page_num, &dialogue_num, &alignment, token_num, token_as_int, token, &text, &color,
                    &page_dialogue, &start_page, &current_page);
                break;
        }
        token = strtok(NULL, ",");
        token_num++;
    }
    if (num_pages > 1) {
        CheckpointPage* final_page;
        initCheckpointPage(&final_page, page_dialogue, current_page, NULL);
        current_page->next_page = final_page;
    } else {
        CheckpointPage* page;
        initCheckpointPage(&page, page_dialogue, NULL, NULL);
        start_page = page;
    }

    initCheckpoint(&checkpoint, x, y, start_page);
    enqueueCheckpoint(game_data->checkpoint_queue, checkpoint);
}

static void setCheckpointPage(int* page_num, int* dialogue_num, int* alignment, const int token_num,
        const int token_as_int, char* token, char** text, SDL_Color* color, DialogueBox* (*page_dialogue)[10],
        CheckpointPage** start_page, CheckpointPage** current_page) {
    const int adjusted_token_num = token_num - 3;
    switch (adjusted_token_num % 6) {
        case 0:
            if (*page_num != token_as_int && *page_num != -1) {
                CheckpointPage* new_page;
                if (*page_num == 0) {
                    initCheckpointPage(&new_page, *page_dialogue, NULL, NULL);
                    *current_page = new_page;
                    *start_page = new_page;
                } else {
                    initCheckpointPage(&new_page, *page_dialogue, *current_page, NULL);
                    (*current_page)->next_page = new_page;
                    *current_page = new_page;
                }
                *current_page = new_page;
                for (int i = 0; i < *dialogue_num; i++) {
                    (*page_dialogue)[i] = NULL;
                }
                *dialogue_num = 0;
            }
            *page_num = token_as_int;
            break;
        case 1:
            *text = token;
            break;
        case 2:
            *alignment = getAlignment(token);
            break;
        case 3:
            color->r = token_as_int;
            break;
        case 4:
            color->g = token_as_int;
            break;
        case 5:
            color->b = token_as_int;
            DialogueBox* box = malloc(sizeof(DialogueBox));
            const SDL_FRect dialogue_rect = getDialogueBoxRect(box, *page_dialogue, *dialogue_num, *text, *alignment);
            initDialogueBox(box, dialogue_rect, *text, *alignment, *color);
            (*page_dialogue)[*dialogue_num] = box;
            *dialogue_num += 1;
            break;
        default:
            break;
    }
}

static int getAlignment(const char* alignment) {
    if (strcmp(alignment, "left") == 0) {
        return ALIGN_LEFT;
    }
    if (strcmp(alignment, "center") == 0) {
        return ALIGN_CENTER;
    }
    return ALIGN_RIGHT;
}

static SDL_FRect getDialogueBoxRect(DialogueBox* box, DialogueBox* page_dialogue[10], const int dialogue_num,
        const char* text, const int alignment) {
    float x = CHECKPOINT_PAGE_X;
    float y = CHECKPOINT_PAGE_Y + DIALOGUE_BOX_SPACING;
    float line_height;
    setLineHeight(&line_height, DIALOGUE_FONT_SIZE, DIALOGUE_LINE_SPACING);
    const int num_lines = getNumLines(text);
    const float h = (float) num_lines * line_height;

    if (alignment == ALIGN_LEFT) {
        x += DIALOGUE_BOX_SPACING;
    } else {
        x += CHECKPOINT_PAGE_WIDTH - DIALOGUE_BOX_WIDTH - DIALOGUE_BOX_SPACING;
    }

    for (int i = 0; i < dialogue_num; i++) {
        y += page_dialogue[i]->label->rect.h + DIALOGUE_BOX_SPACING;
    }

    const SDL_FRect rect = {.x = x, .y = y, .w = DIALOGUE_BOX_WIDTH, .h = h};
    return rect;
}

static void setEntities(GameData* game_data, char line[500], int* file_section) {
    const char *token = strtok(line, ",");
    if (token[0] == '"') {
        *file_section += 1;
        return;
    }
    int token_num = 0;
    int type_index = -1;
    double x = -1;
    double y = -1;
    while (token != NULL && token_num < 4) {
        char* end_token;
        const int token_as_int = strtol(token, &end_token, 10);
        const double token_as_double = strtod(token, &end_token);
        switch (token_num) {
            case 0:
                x = token_as_double * GRID_SIZE + GRID_SIZE / 2.0;
                break;
            case 1:
                y = token_as_double * GRID_SIZE + GRID_SIZE / 2.0;
                break;
            case 2:
                type_index = token_as_int;
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
    game_data->num_entities++;
}

static void setSkyTexture(GameData* game_data, char line[500], int* file_section) {
    const char *token = strtok(line, ",");
    if (token[0] == '"') {
        *file_section += 1;
        return;
    }
    char* end_token;
    const int token_as_int = strtol(token, &end_token, 10);
    game_data->sky_texture_index = token_as_int;
}

static void setPlayer(GameData* game_data, char line[500], int* file_section) {
    initPlayer(&game_data->player);
    const char *token = strtok(line, ",");
    if (token[0] == '"') {
        *file_section += 1;
        return;
    }
    int token_num = 0;
    while (token != NULL && token_num < 3) {
        char* end_token;
        const double token_as_double = strtod(token, &end_token);
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
            default:
                break;
        }
        token = strtok(NULL, ",");
        token_num++;
    }
}