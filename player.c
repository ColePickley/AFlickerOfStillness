#include "player.h"

#include <stdlib.h>
#include <math.h>
#include <SDL3/SDL_timer.h>

#include "functions.h"
#include "program_data.h"

void initKey(Key** key, const int entity_id, const int chunk) {
    *key = malloc(sizeof(Key));
    (*key)->entity_id = entity_id;
    (*key)->chunk = chunk;
}

void destroyKey(Key** key) {
    free(*key);
    *key = NULL;
}

void initPlayer(Player** player) {
    *player = malloc(sizeof(Player));
    if (player == NULL) {
        perror("Failed to allocate memory for player");
        return;
    }
    (*player)->w_pressed = false;
    (*player)->a_pressed = false;
    (*player)->s_pressed = false;
    (*player)->d_pressed = false;
    (*player)->primary_selected = true;
    (*player)->num_keys = 0;
    (*player)->last_update_time = -1;
    (*player)->health = 100;
    (*player)->x_velocity = 0;
    (*player)->y_velocity = 0;
    (*player)->x = 0;
    (*player)->y = 0;
    (*player)->rotation = 0;
    for (int i = 0; i < NUM_RAYS; i++) {
        (*player)->rays[i].x = 0.0;
        (*player)->rays[i].y = 0.0;
        (*player)->rays[i].degree = 0.25 * i - 30.0;
    }
    (*player)->primary_weapon = NULL;
    (*player)->secondary_weapon = NULL;
}

void destroyPlayer(Player** player) {
    if ((*player)->primary_weapon != NULL) {
        destroyWeapon(&(*player)->primary_weapon);
    }
    if ((*player)->secondary_weapon != NULL) {
        destroyWeapon(&(*player)->secondary_weapon);
    }
    free(*player);
    *player = NULL;
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
    if (!isXCollision((SDL_FPoint) {.x = (float) player->x, .y = (float) player->y}, map_grid, delta_x, 5)) {
        player->x += delta_x;
    }
    if (!isYCollision((SDL_FPoint) {.x = (float) player->x, .y = (float) player->y}, map_grid, delta_y, 5)) {
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