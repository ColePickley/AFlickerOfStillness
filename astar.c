#include "astar.h"

#include <stdio.h>
#include <math.h>
#include "functions.h"

void setEntityDirection(Entity* entity, const GameData* game_data) {
    const SDL_FPoint cell_pos = {.x = (float) fmod(entity->x, 64), .y = (float) fmod(entity->y, 64)};
    if (cell_pos.x <= 32 + 4 && cell_pos.x >= 32 - 4 && cell_pos.y <= 32 + 4 && cell_pos.y >= 32 - 4) {
        setEntityPath(entity, game_data, game_data->player);
        if (entity->path == NULL) {
            return;
        }
        entity->x_direction = entity->path->start_node->next_node->pos.x - entity->path->start_node->pos.x;
        entity->y_direction = entity->path->start_node->next_node->pos.y - entity->path->start_node->pos.y;
    }
}

static void setEntityPath(Entity* entity, const GameData* game_data, const Player* player) {
    Node start_node;
    Node end_node;
    initStartAndEndNodes(&start_node, &end_node, entity, player);
    if (start_node.pos.x == end_node.pos.x && start_node.pos.y == end_node.pos.y) {
        entity->path = NULL;
        return;
    }
    Node* (*node_grid)[MAX_MAP_SIZE][MAX_MAP_SIZE] = malloc(sizeof(Node*) * MAX_MAP_SIZE * MAX_MAP_SIZE);
    (*node_grid)[start_node.pos.y][start_node.pos.x] = &start_node;
    closeNode(&start_node, &end_node, node_grid, game_data);
    bool path_found = false;
    int cycle = 0;
    while (!path_found) {
        uint16_t min_f_cost = 65535;
        Node* current_node = NULL;
        for (int y = 0; y < game_data->num_map_rows; y++) {
            for (int x = 0; x < game_data->num_map_cols; x++) {
                const bool valid_node = (*node_grid)[y][x] != NULL && (*node_grid)[y][x]->open;
                if (!valid_node) {
                    continue;
                }
                const bool new_min_f = (*node_grid)[y][x]->f_cost < min_f_cost;
                const bool equal_min_f = (*node_grid)[y][x]->f_cost == min_f_cost;
                bool lower_h;
                if (current_node == NULL) {
                    lower_h = false;
                } else {
                    lower_h = (*node_grid)[y][x]->h_cost < current_node->h_cost;
                }
                if (new_min_f || (equal_min_f && lower_h)) {
                    min_f_cost = (*node_grid)[y][x]->f_cost;
                    current_node = (*node_grid)[y][x];
                }
            }
        }
        if (current_node == NULL) {
            entity->path = NULL;
            return;
        }
        if (current_node->pos.x == end_node.pos.x && current_node->pos.y == end_node.pos.y) {
            path_found = true;
            end_node = *current_node;
        } else {
            closeNode(current_node, &end_node, node_grid, game_data);
        }
        cycle++;
    }
    initEntityPath(&entity->path, &end_node);
}

static void initStartAndEndNodes(Node* start_node, Node* end_node, const Entity* entity, const Player* player) {
    start_node->pos.x = (int) (entity->x / GRID_SIZE);
    start_node->pos.y = (int) (entity->y / GRID_SIZE);
    end_node->pos.x = (int) (player->x / GRID_SIZE);
    end_node->pos.y = (int) (player->y / GRID_SIZE);
    start_node->g_cost = 0;
    end_node->h_cost = 0;
    start_node->parent_node = NULL;
    end_node->parent_node = NULL;

    start_node->h_cost = getHCost(start_node, end_node);
    end_node->g_cost = start_node->h_cost;
    start_node->f_cost = start_node->h_cost;
    end_node->f_cost = end_node->g_cost;
    start_node->open = true;
    end_node->open = false;
}

static void closeNode(Node* node, const Node* end_node, Node* (*node_grid)[MAX_MAP_SIZE][MAX_MAP_SIZE],
        const GameData* game_data) {
    node->open = false;
    addNode(node, end_node, node_grid, game_data, (SDL_Point) {.x = 1, .y = -1});
    addNode(node, end_node, node_grid, game_data, (SDL_Point) {.x = 1, .y = 0});
    addNode(node, end_node, node_grid, game_data, (SDL_Point) {.x = 1, .y = 1});
    addNode(node, end_node, node_grid, game_data, (SDL_Point) {.x = 0, .y = -1});
    addNode(node, end_node, node_grid, game_data, (SDL_Point) {.x = 0, .y = 1});
    addNode(node, end_node, node_grid, game_data, (SDL_Point) {.x = -1, .y = -1});
    addNode(node, end_node, node_grid, game_data, (SDL_Point) {.x = -1, .y = 0});
    addNode(node, end_node, node_grid, game_data, (SDL_Point) {.x = -1, .y = 1});
}

static void addNode(Node* parent, const Node* end_node, Node* (*node_grid)[MAX_MAP_SIZE][MAX_MAP_SIZE],
        const GameData* game_data, const SDL_Point direction) {
    Node* new_node = malloc(sizeof(Node));
    new_node->pos.x = parent->pos.x + direction.x;
    new_node->pos.y = parent->pos.y + direction.y;
    if (new_node->pos.x < 1 || new_node->pos.y < 1 || new_node->pos.x >= game_data->num_map_cols - 1 ||
            new_node->pos.y >= game_data->num_map_rows - 1) {
        return;
    }
    if (game_data->map_grid[new_node->pos.y][new_node->pos.x] > 0) {
        return;
    }
    if ((*node_grid)[new_node->pos.y][new_node->pos.x] != NULL
            && (*node_grid)[new_node->pos.y][new_node->pos.x]->open == false) {
        return;
    }
    new_node->g_cost = parent->g_cost;
    if (direction.x == 0 || direction.y == 0) {
        new_node->g_cost += CARDINAL_DIST;
    } else {
        new_node->g_cost += DIAGONAL_DIST;
    }
    new_node->h_cost = getHCost(new_node, end_node);
    new_node->f_cost = new_node->g_cost + new_node->h_cost;
    new_node->parent_node = parent;
    new_node->open = true;
    (*node_grid)[new_node->pos.y][new_node->pos.x] = new_node;
}

static int getHCost(const Node* new_node, const Node* end_node) {
    const int x_dist = abs(new_node->pos.x - end_node->pos.x);
    const int y_dist = abs(new_node->pos.y - end_node->pos.y);
    return min(x_dist, y_dist) * DIAGONAL_DIST + (max(x_dist, y_dist) - min(x_dist, y_dist)) * CARDINAL_DIST;
}

/*
 * Entity Path Functions
 */

void initEntityPath(EntityPath** path, const Node* end_node) {
    uint16_t size = 0;
    const Node* current_node = end_node;
    while (current_node->parent_node != NULL) {
        size++;
        current_node = current_node->parent_node;
    }
    PathNode* start_path_node = malloc(sizeof(PathNode));
    PathNode* current_path_node = start_path_node;
    for (int i = size; i >= 0; i--) {
        current_node = end_node;
        for (int j = 0; j < i; j++) {
            current_node = current_node->parent_node;
        }
        if (i == size) {
            current_path_node->pos = current_node->pos;
            current_path_node->next_node = NULL;
        } else {
            PathNode* new_node = malloc(sizeof(PathNode));
            new_node->pos = current_node->pos;
            new_node->next_node = NULL;
            current_path_node->next_node = new_node;
            current_path_node = current_path_node->next_node;
        }
    }
    *path = malloc(sizeof(EntityPath));
    (*path)->size = size;
    (*path)->start_node = start_path_node;
}

void destroyEntityPath(EntityPath** path) {
    PathNode* current_path_node = (*path)->start_node;
    for (int i = 0; i < (*path)->size; i++) {
        free(current_path_node);
        current_path_node = current_path_node->next_node;
    }
    free(*path);
    *path = NULL;
}