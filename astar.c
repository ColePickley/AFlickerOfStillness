#include "astar.h"

#include <math.h>
#include <stdio.h>

#include "functions.h"
#include "stdlib.h"

static bool initDefaultNode(Node** node) {
    *node = malloc(sizeof(Node));
    if (node == NULL || *node == NULL) {
        printf("Failed to allocate Node.\n");
        return false;
    }
    (*node)->parent_node = NULL;
    (*node)->pos = (SDL_Point) {.x = -1, .y = -1};
    (*node)->g_cost = 0;
    (*node)->h_cost = 0;
    (*node)->f_cost = 0;
    return true;
}

static void destroyNode(Node** node) {
    if (node == NULL || *node == NULL) {
        printf("Failed to destroy Node.\n");
        return;
    }
    free(*node);
    *node = NULL;
}

void setEntityDirection(Entity* entity, const GameData* game_data) {
    if (entity->last_path_time == 0) {
        entity->last_path_time = SDL_GetTicks();
    }
    const uint64_t time_elapsed = SDL_GetTicks() - entity->last_path_time;
    if (time_elapsed < 1000) return;

    const SDL_Point entity_grid_pos = {
        .x = (int) entity->x / GRID_SIZE,
        .y = (int) entity->y / GRID_SIZE
    };
    const SDL_Point player_grid_pos = {
        .x = (int) game_data->player->x / GRID_SIZE,
        .y = (int) game_data->player->y / GRID_SIZE
    };
    if (entity_grid_pos.x == player_grid_pos.x && entity_grid_pos.y == player_grid_pos.y) {
        const double vectX = entity->x - game_data->player->x;
        const double vectY = (entity->y - game_data->player->y) * -1.0;
        const double rotation = atan2(vectY, vectX) - PI / 2.0;
        entity->x_direction = sin(rotation);
        entity->y_direction = cos(rotation);
        entity->last_path_time = SDL_GetTicks();
        return;
    }

    const SDL_FPoint cell_pos = {
        .x = (float) fmod(entity->x, 64),
        .y = (float) fmod(entity->y, 64)
    };
    if (cell_pos.x <= 32 + 8 && cell_pos.x >= 32 - 8 && cell_pos.y <= 32 + 8 && cell_pos.y >= 32 - 8) {
        setEntityPath(entity, game_data, game_data->player);
        if (entity->path == NULL) {
            entity->last_path_time = SDL_GetTicks();
            return;
        }
        entity->x_direction = entity->path->start_node->next_node->pos.x - entity->path->start_node->pos.x;
        entity->y_direction = entity->path->start_node->next_node->pos.y - entity->path->start_node->pos.y;
    }
    entity->last_path_time = SDL_GetTicks();
}

static void setEntityPath(Entity* entity, const GameData* game_data, const Player* player) {
    if (entity->path != NULL) destroyEntityPath(&entity->path);
    Node* start_node;
    Node* end_node;
    if (!initDefaultNode(&start_node) || !initDefaultNode(&end_node)) {
        destroyNode(&start_node);
        destroyNode(&end_node);
        return;
    }
    setStartAndEndNodes(start_node, end_node, entity, player);
    if (start_node->pos.x == end_node->pos.x && start_node->pos.y == end_node->pos.y) {
        destroyNode(&start_node);
        destroyNode(&end_node);
        return;
    }
    SDL_Point closed_nodes[MAX_NUM_NODES];
    uint16_t num_closed_nodes = 0;
    MinHeap* open_nodes;
    initMinHeap(&open_nodes);
    closeNode(start_node, end_node, open_nodes, &closed_nodes, &num_closed_nodes, game_data);
    bool path_found = false;
    int cycle = 0;
    while (!path_found) {
        Node* current_node = popNode(open_nodes);
        if (current_node == NULL) {
            destroyMinHeap(&open_nodes);
            destroyNode(&end_node);
            return;
        }
        if (current_node->pos.x == end_node->pos.x && current_node->pos.y == end_node->pos.y) {
            path_found = true;
            destroyNode(&end_node);
            end_node = current_node;
        } else if (closeNode(current_node, end_node, open_nodes, &closed_nodes, &num_closed_nodes, game_data)) {
            destroyMinHeap(&open_nodes);
            destroyNode(&end_node);
            return;
        }
        cycle++;
    }
    initEntityPath(&entity->path, end_node);
    destroyMinHeap(&open_nodes);
}

static void setStartAndEndNodes(Node* start_node, Node* end_node, const Entity* entity, const Player* player) {
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
}

static bool closeNode(Node* node, const Node* end_node, MinHeap* heap, SDL_Point (*closed_nodes)[MAX_NUM_NODES],
        uint16_t* num_closed_nodes, const GameData* game_data) {
    *num_closed_nodes += 1;
    if (*num_closed_nodes >= MAX_NUM_NODES) return true;
    (*closed_nodes)[*num_closed_nodes - 1] = node->pos;
    bool heap_full = false;
    if (addNode(node, end_node, heap, closed_nodes, num_closed_nodes, game_data, (SDL_Point) {.x = 1, .y = -1}))
        heap_full = true;
    if (addNode(node, end_node, heap, closed_nodes, num_closed_nodes, game_data, (SDL_Point) {.x = 1, .y = 0}))
        heap_full = true;
    if (addNode(node, end_node, heap, closed_nodes, num_closed_nodes, game_data, (SDL_Point) {.x = 1, .y = 1}))
        heap_full = true;
    if (addNode(node, end_node, heap, closed_nodes, num_closed_nodes, game_data, (SDL_Point) {.x = 0, .y = -1}))
        heap_full = true;
    if (addNode(node, end_node, heap, closed_nodes, num_closed_nodes, game_data, (SDL_Point) {.x = 0, .y = 1}))
        heap_full = true;
    if (addNode(node, end_node, heap, closed_nodes, num_closed_nodes, game_data, (SDL_Point) {.x = -1, .y = -1}))
        heap_full = true;
    if (addNode(node, end_node, heap, closed_nodes, num_closed_nodes, game_data, (SDL_Point) {.x = -1, .y = 0}))
        heap_full = true;
    if (addNode(node, end_node, heap, closed_nodes, num_closed_nodes, game_data, (SDL_Point) {.x = -1, .y = 1}))
        heap_full = true;
    return heap_full;
}

static bool addNode(Node* parent, const Node* end_node, MinHeap* heap, const SDL_Point (*closed_nodes)[MAX_NUM_NODES],
        const uint16_t* num_closed_nodes, const GameData* game_data, const SDL_Point direction) {
    const SDL_Point new_node_pos = {.x = parent->pos.x + direction.x, .y = parent->pos.y + direction.y};
    if (new_node_pos.x < 1 || new_node_pos.y < 1 || new_node_pos.x >= game_data->num_map_cols - 1 ||
            new_node_pos.y >= game_data->num_map_rows - 1) {
        return false;
    }
    if (game_data->map_grid[new_node_pos.y][new_node_pos.x] > 0) {
        return false;
    }
    for (int i = 0; i < *num_closed_nodes; i++) {
        if (new_node_pos.x == (*closed_nodes)[i].x && new_node_pos.y == (*closed_nodes)[i].y) {
            return false;
        }
    }
    if (direction.x != 0 && direction.y != 0 && (game_data->map_grid[parent->pos.y][parent->pos.x + direction.x] > 0 ||
            game_data->map_grid[parent->pos.y + direction.y][parent->pos.x] > 0)) {
        return false;
    }
    Node* new_node;
    initDefaultNode(&new_node);
    new_node->pos = new_node_pos;
    int new_g_cost = parent->g_cost;
    if (direction.x == 0 || direction.y == 0) {
        new_g_cost += CARDINAL_DIST;
    } else {
        new_g_cost += DIAGONAL_DIST;
    }
    new_node->g_cost = new_g_cost;
    new_node->h_cost = getHCost(new_node, end_node);
    new_node->f_cost = new_node->g_cost + new_node->h_cost;
    new_node->parent_node = parent;
    return !pushNode(heap, new_node);
}

static int getHCost(const Node* new_node, const Node* end_node) {
    const int x_dist = abs(new_node->pos.x - end_node->pos.x);
    const int y_dist = abs(new_node->pos.y - end_node->pos.y);
    return min(x_dist, y_dist) * DIAGONAL_DIST + (max(x_dist, y_dist) - min(x_dist, y_dist)) * CARDINAL_DIST;
}

/*
 *----------------------------------------------------------------------------------------------------------------------
 * Entity Path Functions
 *----------------------------------------------------------------------------------------------------------------------
 */

bool initEntityPath(EntityPath** path, const Node* end_node) {
    *path = malloc(sizeof(EntityPath));
    if (*path == NULL) {
        printf("Error allocating memory for EntityPath.\n");
        return false;
    }
    uint16_t size = 0;
    const Node* current_node = end_node;
    while (current_node->parent_node != NULL) {
        size++;
        current_node = current_node->parent_node;
    }
    PathNode* start_path_node = malloc(sizeof(PathNode));
    if (start_path_node == NULL) {
        printf("Error allocating memory for start_path_node.\n");
        destroyEntityPath(path);
        return false;
    }
    (*path)->size = size;
    (*path)->start_node = start_path_node;
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
            if (new_node == NULL) {
                printf("Error allocating memory for new_node.\n");
                destroyEntityPath(path);
                return false;
            }
            new_node->pos = current_node->pos;
            new_node->next_node = NULL;
            current_path_node->next_node = new_node;
            current_path_node = current_path_node->next_node;
        }
    }
    return true;
}

void destroyEntityPath(EntityPath** path) {
    if (path == NULL || *path == NULL) {
        printf("Error destroying entity path.\n");
        return;
    }
    PathNode* current = (*path)->start_node;
    while (current != NULL) {
        PathNode* next = current->next_node;
        free(current);
        current = next;
    }
    free(*path);
    *path = NULL;
}

/*
 *----------------------------------------------------------------------------------------------------------------------
 * Min Heap Functions
 *----------------------------------------------------------------------------------------------------------------------
 */

bool initMinHeap(MinHeap** min_heap) {
    *min_heap = malloc(sizeof(MinHeap));
    if (*min_heap == NULL) {
        printf("Error allocating memory for MinHeap.\n");
        return false;
    }
    for (int i = 0; i < MAX_NUM_NODES; i++) {
        (*min_heap)->nodes[i] = NULL;
    }
    (*min_heap)->size = 0;
    return true;
}

void destroyMinHeap(MinHeap** min_heap) {
    for (int i = 0; i < (*min_heap)->size; i++) {
        if ((*min_heap)->nodes[i] != NULL) {
            free((*min_heap)->nodes[i]);
        }
    }
    free(*min_heap);
    *min_heap = NULL;
}

bool pushNode(MinHeap* min_heap, Node* new_node) {
    if (min_heap->size == MAX_NUM_NODES) { printf("Heap is full.\n"); return false; }
    int i = min_heap->size++;
    min_heap->nodes[i] = new_node;
    while (i > 0) {
        const int parent = (i - 1) / 2;
        if (min_heap->nodes[i]->f_cost >= min_heap->nodes[parent]->f_cost) break;
        swap_nodes(&min_heap->nodes[i], &min_heap->nodes[parent]);
        i = parent;
    }
    return true;
}

Node* popNode(MinHeap* min_heap) {
    if (min_heap->size == 0) return NULL;
    Node* min = min_heap->nodes[0];
    min_heap->size--;
    min_heap->nodes[0] = min_heap->nodes[min_heap->size];
    min_heap->nodes[min_heap->size] = NULL;          // avoid stale pointers (double free later)
    int i = 0;
    while (true) {
        const int l = 2 * i + 1;
        const int r = 2 * i + 2;
        int s = i;
        if (l < min_heap->size && min_heap->nodes[l]->f_cost < min_heap->nodes[s]->f_cost) s = l;
        if (r < min_heap->size && min_heap->nodes[r]->f_cost < min_heap->nodes[s]->f_cost) s = r;
        if (s == i) break;
        swap_nodes(&min_heap->nodes[i], &min_heap->nodes[s]);
        i = s;
    }
    return min;
}

Node* peekNode(const MinHeap* min_heap) {
    if (min_heap->size == 0) {
        return NULL;
    }
    return min_heap->nodes[0];
}

void swap_nodes(Node** node1, Node** node2) {
    Node *temp = *node1;
    *node1 = *node2;
    *node2 = temp;
}