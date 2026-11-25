#include "program_data.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "functions.h"

void initGameData(GameData** game_data) {
    *game_data = malloc(sizeof(GameData));
    if (*game_data == NULL) {
        printf("Failed to allocate game data.\n");
        return;
    }
    (*game_data)->level_index = 0;
    (*game_data)->save_index = 0;
    (*game_data)->num_hit_checkpoints = 0;
    (*game_data)->num_map_rows = 0;
    (*game_data)->num_map_cols = 0;
    for (int i = 0; i < MAX_MAP_SIZE; i++) {
        for (int j = 0; j < MAX_MAP_SIZE; j++) {
            (*game_data)->map_grid[i][j] = 0;
            (*game_data)->chunk_grid[i][j] = 0;
            for (int k = 0; k < 6; k++) {
                (*game_data)->texture_locations[i][j][k] = 0;
            }
        }
    }
    (*game_data)->num_entities = 0;
    for (int i = 0; i < MAX_NUM_ENTITIES; i++) {
        (*game_data)->entities[i] = NULL;
    }
    CheckpointQueue* checkpoint_queue;
    initCheckpointQueue(&checkpoint_queue);
    (*game_data)->checkpoint_queue = checkpoint_queue;
    (*game_data)->current_checkpoint = NULL;
    (*game_data)->last_checkpoint = NULL;
    (*game_data)->player = NULL;
}

void destroyGameData(GameData** game_data) {
    if (*game_data == NULL) return;
    destroyCheckpointQueue(&(*game_data)->checkpoint_queue);
    if ((*game_data)->player != NULL) {
        destroyPlayer(&(*game_data)->player);
    }
    for (int i = 0; i < (*game_data)->num_entities; i++) {
        destroyEntity(&(*game_data)->entities[i]);
    }
    free(*game_data);
    *game_data = NULL;
}

void initUserData(UserData** user_data) {
    *user_data = malloc(sizeof(UserData));
    if (*user_data == NULL) {
        printf("Failed to allocate user data.\n");
        return;
    }
    (*user_data)->frame_rate_cap = 60;
    (*user_data)->fullscreen = false;
    (*user_data)->graphics_quality = GRAPHICS_QUALITY_LOW;
    (*user_data)->sensitivity = 50;
}

void resetUserData(UserData* user_data) {
    user_data->frame_rate_cap = 60;
    user_data->fullscreen = false;
    user_data->graphics_quality = GRAPHICS_QUALITY_LOW;
    user_data->sensitivity = 50;
}

void destroyUserData(UserData** user_data) {
    if (*user_data == NULL) return;
    free(*user_data);
    *user_data = NULL;
}

void addEntity(GameData* game_data, Entity* entity) {
    if (game_data->num_entities >= MAX_NUM_ENTITIES) {
        return;
    }
    game_data->num_entities++;
    game_data->entities[game_data->num_entities - 1] = entity;
}

void removeEntity(GameData* game_data, const int entity_index) {
    if (entity_index < 0 || entity_index >= game_data->num_entities) {
        return;
    }
    destroyEntity(&game_data->entities[entity_index]);
    game_data->entities[entity_index] = NULL;
    for (int i = entity_index; i < game_data->num_entities - 1; i++) {
        game_data->entities[i] = game_data->entities[i + 1];
    }
    game_data->num_entities--;
    game_data->entities[game_data->num_entities] = NULL;
}


/*
 *----------------------------------------------------------------------------------------------------------------------
 * Entity Collection Functions
 *----------------------------------------------------------------------------------------------------------------------
 */

bool collectAmmo(const GameData* game_data, const int entity_index) {
    if (game_data->player->primary_weapon == NULL) return false;
    if (game_data->player->primary_weapon->mag_count == game_data->player->primary_weapon->max_mag_count) return false;
    if (game_data->entities[entity_index]->ammo_type == game_data->player->primary_weapon->ammo_type) {
        game_data->player->primary_weapon->mag_count += game_data->entities[entity_index]->mag_amount;
        game_data->player->primary_weapon->mag_count =
            min(game_data->player->primary_weapon->max_mag_count,
            game_data->player->primary_weapon->mag_count);
        return true;
    }
    return false;
}

bool collectWeapon(GameData* game_data, const int entity_index) {
    if (!game_data->player->pickup_enabled) return false;
    Weapon* new_weapon;
    initWeapon(&new_weapon, game_data->entities[entity_index]->weapon_index,
        game_data->entities[entity_index]->texture_id, game_data->entities[entity_index]->mag_count,
        game_data->entities[entity_index]->ammo_count);
    Weapon* old_weapon;
    if (isPrimaryWeapon(new_weapon)) {
        old_weapon = game_data->player->primary_weapon;
        game_data->player->primary_weapon = new_weapon;
    } else {
        old_weapon = game_data->player->secondary_weapon;
        game_data->player->secondary_weapon = new_weapon;
    }
    if (old_weapon != NULL) {
        Entity* new_entity;
        initDefaultEntity(&new_entity, old_weapon->entity_id, game_data->entities[entity_index]->x,
            game_data->entities[entity_index]->y);
        new_entity->mag_count = old_weapon->mag_count;
        new_entity->ammo_count = old_weapon->ammo_count;
        addEntity(game_data, new_entity);
        game_data->player->pickup_enabled = false;
    }
    removeEntity(game_data, entity_index);
    return true;
}

bool collectHealth(GameData* game_data, const int entity_index) {
    if (game_data->player->health < MAX_PLAYER_HEALTH) {
        game_data->player->health += game_data->entities[entity_index]->health_amount;
        game_data->player->health = min(game_data->player->health, MAX_PLAYER_HEALTH);
        removeEntity(game_data, entity_index);
        return true;
    }
    return false;
}

void collectKey(GameData* game_data, const int entity_index) {
    if (game_data->player->num_keys < MAX_NUM_KEYS) {
        const int entity_id = game_data->entities[entity_index]->texture_id;
        const int grid_y = (int) (game_data->entities[entity_index]->y / 64);
        const int grid_x = (int) (game_data->entities[entity_index]->x / 64);
        const int chunk = game_data->chunk_grid[grid_y][grid_x];
        Key* new_key;
        initKey(&new_key, entity_id, chunk);
        game_data->player->keys[game_data->player->num_keys] = new_key;
        game_data->player->num_keys++;
        removeEntity(game_data, entity_index);
    }
}

/*
 *----------------------------------------------------------------------------------------------------------------------
 * Collision Checking
 *----------------------------------------------------------------------------------------------------------------------
 */

bool playerEntityCollisionCheck(GameData* game_data, const int entity_index) {
    const bool is_collision = fabs(game_data->entities[entity_index]->x - game_data->player->x) < 10 &&
        fabs(game_data->entities[entity_index]->y - game_data->player->y) < 10;
    if (!is_collision) return false;
    switch (game_data->entities[entity_index]->subtype_id) {
        case ENTITY_SUBTYPE_AESTHETIC:
        case ENTITY_SUBTYPE_ENEMY_PROJECTILE:
            return false;
        case ENTITY_SUBTYPE_COLLECTABLE_AMMO:
            return collectAmmo(game_data, entity_index);
        case ENTITY_SUBTYPE_COLLECTABLE_HEALTH:
            return collectHealth(game_data, entity_index);
        case ENTITY_SUBTYPE_COLLECTABLE_WEAPON:
            return collectWeapon(game_data, entity_index);
        case ENTITY_SUBTYPE_COLLECTABLE_KEY:
            collectKey(game_data, entity_index);
            return true;
        case ENTITY_SUBTYPE_ENEMY_MELEE:
        case ENTITY_SUBTYPE_ENEMY_MELEE_AND_PROJECTILE:
            game_data->player->health -= enemyMeleeAttack(game_data->entities[entity_index]);
            return false;
        case ENTITY_SUBTYPE_PROJECTILE:
            game_data->player->health -= game_data->entities[entity_index]->projectile_damage;
            removeEntity(game_data, entity_index);
            return true;
        default:
            return false;
    }
}

bool entityEntityCollisionCheck(GameData* game_data, const int entity_index1, const int entity_index2) {
    const double max_collision_distance =
        (game_data->entities[entity_index1]->scale + game_data->entities[entity_index2]->scale) / 8.0;
    const double x_distance = fabs(game_data->entities[entity_index2]->x - game_data->entities[entity_index1]->x);
    const double y_distance = fabs(game_data->entities[entity_index2]->y - game_data->entities[entity_index1]->y);
    const bool is_collision = x_distance <= max_collision_distance && y_distance <= max_collision_distance;
    if (!is_collision) return false;
    const int type1 = getEntityType(game_data->entities[entity_index1]);
    const int type2 = getEntityType(game_data->entities[entity_index2]);
    if (type1 == ENTITY_TYPE_PROJECTILE && type2 == ENTITY_TYPE_ENEMY) {
        game_data->entities[entity_index2]->health -= game_data->entities[entity_index1]->projectile_damage;
        removeEntity(game_data, entity_index1);
        return true;
    }
    return false;
}