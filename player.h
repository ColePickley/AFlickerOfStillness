#ifndef AFLICKEROFSTILLNESS_PLAYER_H
#define AFLICKEROFSTILLNESS_PLAYER_H
#include <stdbool.h>
#include <stdint.h>
#include "constants.h"

typedef struct {
    double x;
    double y;
    double degree;
    double length;
    bool vert_collision;
}Ray;

typedef struct {
    bool w_pressed;
    bool a_pressed;
    bool s_pressed;
    bool d_pressed;
    uint64_t last_update_time;
    double x_velocity;
    double y_velocity;
    double x;
    double y;
    double rotation;
    int health;
    int primary_weapon_index;
    int ammo_count;
    int mag_count;
    int secondary_weapon_index;
    Ray* rays[NUM_RAYS];
}Player;

void initPlayer(Player** player);

void destroyPlayer(Player* player);

void movePlayer(Player* player, int (*map_grid)[MAX_MAP_SIZE][MAX_MAP_SIZE]);

void rotatePlayer(Player* player, int sensitivity, double mouse_delta_x);

static void setPlayerVelocity(Player* player);

static bool isXCollision(const Player* player, int (*map_grid)[MAX_MAP_SIZE][MAX_MAP_SIZE], double delta_x);

static bool isYCollision(const Player* player, int (*map_grid)[MAX_MAP_SIZE][MAX_MAP_SIZE], double delta_y);

#endif //AFLICKEROFSTILLNESS_PLAYER_H
