#include "player.h"

#include <stdlib.h>
#include <math.h>
#include <SDL3/SDL_timer.h>

#include "functions.h"
#include "program_data.h"

void initPlayer(Player* player) {
    player->w_pressed = false;
    player->a_pressed = false;
    player->s_pressed = false;
    player->d_pressed = false;
    player->last_update_time = -1;
    player->x_velocity = 0;
    player->y_velocity = 0;
    player->x = 0;
    player->y = 0;
    player->rotation = 0;
    for (int i = 0; i < NUM_RAYS; i++) {
        // player->rays[i] = malloc(sizeof(Ray));
        player->rays[i].x = 0.0;
        player->rays[i].y = 0.0;
        player->rays[i].degree = 0.25 * i - 30.0;
    }
}

void destroyPlayer(Player* player) {
    // for (int i = 0; i < NUM_RAYS; i++) {
    //     if (player->rays[i] != NULL) {
    //         free(player->rays[i]);
    //         player->rays[i] = NULL;
    //     }
    // }
}

void movePlayer(Player* player, int (*map_grid)[MAX_MAP_SIZE][MAX_MAP_SIZE]) {
    setPlayerVelocity(player);
    if (player->last_update_time == -1) {
        player->last_update_time = SDL_GetTicks();
    }
    const uint64_t time_elapsed = SDL_GetTicks() - player->last_update_time;
    const double speed = (double) time_elapsed / 15.0;
    const double delta_x = player->x_velocity * speed;
    const double delta_y = player->y_velocity * speed;
    if (!isXCollision(player, map_grid, delta_x)) {
        player->x += delta_x;
    }
    if (!isYCollision(player, map_grid, delta_y)) {
        player->y += delta_y;
    }
    player->last_update_time = SDL_GetTicks();
}

void rotatePlayer(Player* player, const int sensitivity, const double mouse_delta_x) {
    if (mouse_delta_x > 50 || mouse_delta_x < -50) {
        return;
    }
    player->rotation += mouse_delta_x * ((double) sensitivity / 100.0);
    while (player->rotation >= 360.0) {
        player->rotation -= 360.0;
    }
    while (player->rotation < 0.0) {
        player->rotation += 360.0;
    }
}

static void setPlayerVelocity(Player* player) {
    if (player->w_pressed && player->s_pressed || player->a_pressed && player->d_pressed ||
            !player->w_pressed && !player->a_pressed && !player->s_pressed && !player->d_pressed) {
        player->x_velocity = 0.0;
        player->y_velocity = 0.0;
    } else if (player->w_pressed && player->a_pressed) {
        player->x_velocity = -sin(toRadians(player->rotation + 135));
        player->y_velocity = -cos(toRadians(player->rotation + 135));
    } else if (player->w_pressed && player->d_pressed) {
        player->x_velocity = sin(toRadians(player->rotation + 45));
        player->y_velocity = cos(toRadians(player->rotation + 45));
    } else if (player->s_pressed && player->a_pressed) {
        player->x_velocity = -sin(toRadians(player->rotation + 45));
        player->y_velocity = -cos(toRadians(player->rotation + 45));
    } else if (player->s_pressed && player->d_pressed) {
        player->x_velocity = sin(toRadians(player->rotation + 135));
        player->y_velocity = cos(toRadians(player->rotation + 135));
    } else if (player->w_pressed) {
        player->x_velocity = sin(toRadians(player->rotation));
        player->y_velocity = cos(toRadians(player->rotation));
    } else if (player->a_pressed) {
        player->x_velocity = -sin(toRadians(player->rotation + 90));
        player->y_velocity = -cos(toRadians(player->rotation + 90));
    } else if (player->s_pressed) {
        player->x_velocity = -sin(toRadians(player->rotation));
        player->y_velocity = -cos(toRadians(player->rotation));
    } else if (player->d_pressed) {
        player->x_velocity = sin(toRadians(player->rotation + 90));
        player->y_velocity = cos(toRadians(player->rotation + 90));
    }
}

static bool isXCollision(const Player* player, int (*map_grid)[MAX_MAP_SIZE][MAX_MAP_SIZE], const double delta_x) {
    double adjusted_delta_x = delta_x;
    if (delta_x < 0) {
        adjusted_delta_x -= 5;
    } else if (delta_x > 0) {
        adjusted_delta_x += 5;
    }
    const int grid_x = (int) ((player->x + adjusted_delta_x) / 64);
    const int grid_y = (int) (player->y / 64);
    if ((*map_grid)[grid_y][grid_x] == 1) {
        return true;
    }
    if ((*map_grid)[grid_y][grid_x] == 2) {
        (*map_grid)[grid_y][grid_x] = -2;
    }
    return false;
}

static bool isYCollision(const Player* player, int (*map_grid)[MAX_MAP_SIZE][MAX_MAP_SIZE], const double delta_y) {
    double adjusted_delta_y = delta_y;
    if (delta_y < 0) {
        adjusted_delta_y -= 5;
    } else if (delta_y > 0) {
        adjusted_delta_y += 5;
    }
    const int grid_x = (int) (player->x / 64);
    const int grid_y = (int) ((player->y + adjusted_delta_y) / 64);
    if ((*map_grid)[grid_y][grid_x] == 1) {
        return true;
    }
    if ((*map_grid)[grid_y][grid_x] == 2) {
        (*map_grid)[grid_y][grid_x] = -2;
    }
    return false;
}