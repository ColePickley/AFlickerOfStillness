#ifndef AFLICKEROFSTILLNESS_PLAYER_H
#define AFLICKEROFSTILLNESS_PLAYER_H
#include <stdbool.h>
#include <stdint.h>
#include "constants.h"
#include "weapon.h"

#define MAX_PLAYER_HEALTH 100

typedef struct {
    double x;
    double y;
    double degree;
    double length;
    bool vert_collision;
}Ray;

typedef struct {
    int entity_id;
    int chunk;
}Key;

typedef struct {
    bool w_pressed;
    bool a_pressed;
    bool s_pressed;
    bool d_pressed;
    bool primary_selected;
    uint8_t num_keys;
    uint64_t last_update_time;
    int health;
    double x_velocity;
    double y_velocity;
    double x;
    double y;
    double rotation;
    Ray rays[NUM_RAYS];
    Key* keys[MAX_NUM_KEYS];
    Weapon* primary_weapon;
    Weapon* secondary_weapon;
}Player;

void initKey(Key** key, int entity_id, int chunk);

void destroyKey(Key** key);

void initPlayer(Player** player);

void destroyPlayer(Player** player);

void movePlayer(Player* player, int (*map_grid)[MAX_MAP_SIZE][MAX_MAP_SIZE]);

void rotatePlayer(Player* player, int sensitivity, double mouse_delta_x);

static void setPlayerVelocity(Player* player);

#endif //AFLICKEROFSTILLNESS_PLAYER_H
