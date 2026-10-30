#include "player.h"

#include <stdlib.h>
#include <math.h>
#include <SDL3/SDL_timer.h>
#include "functions.h"

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
    (*player)->attacking = false;
    (*player)->pickup_enabled = true;
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
    const int grid_x = (int) player->x / 64;
    const int grid_y = (int) player->y / 64;
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
    if (((int) player->y / 64 != grid_y || (int) player->x / 64 != grid_x) && !player->pickup_enabled) {
        player->pickup_enabled = true;
    }
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

Entity* playerRangedAttack(const Player* player, Entity* const (*entities)[MAX_NUM_ENTITIES], const int num_entities) {
    if ((SDL_GetTicks() - player->primary_weapon->last_attack_time) / 40 < player->primary_weapon->attack_cool_down) {
        return NULL;
    }
    if (player->primary_weapon->ammo_count == 0) {
        return NULL;
    }
    player->primary_weapon->ammo_count--;
    if (player->primary_weapon->type == WEAPON_PROJECTILE) {
        const double rotation = player->rotation;
        const double x = player->x + sin(toRadians(rotation)) * 20;
        const double y = player->y + cos(toRadians(rotation)) * 20;
        Entity* entity;
        initDefaultEntity(&entity, player->primary_weapon->projectile_index, x, y);
        entity->x_direction = -sin(toRadians(rotation));
        entity->y_direction = -cos(toRadians(rotation));
        player->primary_weapon->last_attack_time = SDL_GetTicks();
        return entity;
    }
    for (int i = 0; i < num_entities; i++) {
        if ((*entities)[i]->in_scope && (*entities)[i]->vect_mag <= player->primary_weapon->range) {
            (*entities)[i]->health -= player->primary_weapon->damage;
        }
    }
    player->primary_weapon->last_attack_time = SDL_GetTicks();
    return NULL;
}

void playerMeleeAttack(const Player* player, Entity* const (*entities)[MAX_NUM_ENTITIES], const int num_entities) {
    for (int i = 0; i < num_entities; i++) {
        if ((*entities)[i]->in_scope && (*entities)[i]->vect_mag <= player->secondary_weapon->range) {
            (*entities)[i]->health -= player->secondary_weapon->damage;
        }
    }
}

void playerReload(const Player* player) {
    if (player->primary_weapon == NULL || !player->primary_selected || player->primary_weapon->mag_count == 0 ||
        player->primary_weapon->ammo_count == player->primary_weapon->max_ammo_count) return;
    player->primary_weapon->ammo_count = player->primary_weapon->max_ammo_count;
    player->primary_weapon->mag_count--;
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