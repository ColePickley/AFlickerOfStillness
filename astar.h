#ifndef AFLICKEROFSTILLNESS_ASTAR_H
#define AFLICKEROFSTILLNESS_ASTAR_H
#include <stdbool.h>
#include <stdint.h>

#include "program_data.h"

#define CARDINAL_DIST 10
#define DIAGONAL_DIST 14

#define MAX_NUM_NODES 128

typedef struct Node Node;

struct Node {
    Node* parent_node;
    SDL_Point pos;
    uint16_t g_cost; // how far away this node is from the start node
    uint16_t h_cost; // how far away this node is from the end node
    uint16_t f_cost; // g_cost + h_cost
};

typedef struct {
    Node* nodes[MAX_NUM_NODES];
    uint16_t size;
}MinHeap;

static bool initDefaultNode(Node** node);

static void destroyNode(Node** node);

void setEntityDirection(Entity* entity, const GameData* game_data);

static void setEntityPath(Entity* entity, const GameData* game_data, const Player* player);

static void setStartAndEndNodes(Node* start_node, Node* end_node, const Entity* entity, const Player* player);

// returns true if the max number of nodes has been reached
static bool closeNode(Node* node, const Node* end_node, MinHeap* heap, SDL_Point (*closed_nodes)[MAX_NUM_NODES],
    uint16_t* num_closed_nodes, const GameData* game_data);

// returns true if the max number of nodes has been reached
static bool addNode(Node* parent, const Node* end_node, MinHeap* heap, const SDL_Point (*closed_nodes)[MAX_NUM_NODES],
    const uint16_t* num_closed_nodes, const GameData* game_data, SDL_Point direction);

static int getHCost(const Node* new_node, const Node* end_node);

/*
 *----------------------------------------------------------------------------------------------------------------------
 * Entity Path Functions
 *----------------------------------------------------------------------------------------------------------------------
 */

bool initEntityPath(EntityPath** path, const Node* end_node);

void destroyEntityPath(EntityPath** path);

/*
 *----------------------------------------------------------------------------------------------------------------------
 * Min Heap Functions
 *----------------------------------------------------------------------------------------------------------------------
 */

bool initMinHeap(MinHeap** min_heap);

void destroyMinHeap(MinHeap** min_heap);

bool pushNode(MinHeap* min_heap, Node* new_node);

Node* popNode(MinHeap* min_heap);

Node* peekNode(const MinHeap* min_heap);

void swap_nodes(Node** node1, Node** node2);

#endif //AFLICKEROFSTILLNESS_ASTAR_H
