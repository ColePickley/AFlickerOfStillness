#ifndef AFLICKEROFSTILLNESS_ASTAR_H
#define AFLICKEROFSTILLNESS_ASTAR_H
#include <stdbool.h>
#include <stdint.h>

#include "program_data.h"

#define CARDINAL_DIST 10
#define DIAGONAL_DIST 14

typedef struct Node Node;

struct Node {
    Node* parent_node;
    SDL_Point pos;
    uint16_t g_cost; // how far away this node is from the start node
    uint16_t h_cost; // how far away this node is from the end node
    uint16_t f_cost; // g_cost + h_cost
    bool open;
};

void setEntityDirection(Entity* entity, const GameData* game_data);

static void setEntityPath(Entity* entity, const GameData* game_data, const Player* player);

static void initStartAndEndNodes(Node* start_node, Node* end_node, const Entity* entity, const Player* player);

static void closeNode(Node* node, const Node* end_node, Node* (*node_grid)[MAX_MAP_SIZE][MAX_MAP_SIZE],
    const GameData* game_data);

static void addNode(Node* parent, const Node* end_node, Node* (*node_grid)[MAX_MAP_SIZE][MAX_MAP_SIZE],
    const GameData* game_data, SDL_Point direction);

static int getHCost(const Node* new_node, const Node* end_node);

/*
 *----------------------------------------------------------------------------------------------------------------------
 * Entity Path Functions
 *----------------------------------------------------------------------------------------------------------------------
 */

void initEntityPath(EntityPath** path, const Node* end_node);

void destroyEntityPath(EntityPath** path);

#endif //AFLICKEROFSTILLNESS_ASTAR_H
