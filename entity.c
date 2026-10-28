#include "entity.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL_timer.h>
#include "functions.h"

void initDefaultEntity(Entity** entity, const int texture_id, const double x, const double y) {
    *entity = malloc(sizeof(Entity));
    // universal data
    (*entity)->x = x;
    (*entity)->y = y;
    (*entity)->vect_x = 0.0;
    (*entity)->vect_y = 0.0;
    (*entity)->vect_mag = 0.0;
    (*entity)->subtype_id = -1;
    (*entity)->texture_id = texture_id;
    (*entity)->last_update_time = 0;
    // type-specific data
    setEntityData(*entity);
    (*entity)->ammo_count = -1;
    (*entity)->mag_count = -1;
    (*entity)->in_range = false;
    (*entity)->in_scope = false;
    // movement
    (*entity)->path = NULL;
    (*entity)->x_direction = 0;
    (*entity)->y_direction = 0;
}

void destroyEntity(Entity** entity) {
    free(*entity);
    *entity = NULL;
}

int getEntityType(const Entity* entity) {
    switch (entity->subtype_id) {
        case ENTITY_SUBTYPE_AESTHETIC:
            return ENTITY_TYPE_AESTHETIC;
        case ENTITY_SUBTYPE_COLLECTABLE_HEALTH:
        case ENTITY_SUBTYPE_COLLECTABLE_WEAPON:
        case ENTITY_SUBTYPE_COLLECTABLE_AMMO:
        case ENTITY_SUBTYPE_COLLECTABLE_KEY:
            return ENTITY_TYPE_COLLECTABLE;
        case ENTITY_SUBTYPE_PROJECTILE:
            return ENTITY_TYPE_PROJECTILE;
        case ENTITY_SUBTYPE_ENEMY_MELEE:
        case ENTITY_SUBTYPE_ENEMY_PROJECTILE:
        case ENTITY_SUBTYPE_ENEMY_MELEE_AND_PROJECTILE:
            return ENTITY_TYPE_ENEMY;
        default:
            return -1;
    }
}

void moveEnemy(Entity* entity, int (*map_grid)[MAX_MAP_SIZE][MAX_MAP_SIZE], Entity* entities[MAX_NUM_ENTITIES],
        const int num_entities) {
    if (entity->last_update_time == -1) {
        entity->last_update_time = SDL_GetTicks();
    }
    if (entity->path == NULL || (entity->x_direction == 0 && entity->y_direction == 0)) {
        entity->last_update_time = SDL_GetTicks();
        return;
    }
    const uint64_t time_elapsed = SDL_GetTicks() - entity->last_update_time;
    const double speed = (double) time_elapsed / 300.0;
    const double delta_x = entity->x_direction * speed * entity->speed;
    const double delta_y = entity->y_direction * speed * entity->speed;
    // check if there is another entity in this entity's next cell
    for (int i = 0; i < num_entities; i++) {
        if (entities[i] == entity || getEntityType(entities[i]) != ENTITY_TYPE_ENEMY) {
            continue;
        }
        if ((int) entities[i]->x / 64 == entity->path->start_node->next_node->pos.x &&
                (int) entities[i]->y / 64 == entity->path->start_node->next_node->pos.y) {
            entity->last_update_time = SDL_GetTicks();
            return;
        }
    }
    // check for wall collisions and move entity
    const SDL_FPoint entity_position = (SDL_FPoint) {.x = (float) entity->x, .y = (float) entity->y};
    if (!isXCollision(entity_position, map_grid, delta_x, 32)) {
        entity->x += delta_x;
    }
    if (!isYCollision(entity_position, map_grid, delta_y, 32)) {
        entity->y += delta_y;
    }
    entity->last_update_time = SDL_GetTicks();
}

void moveProjectile(Entity* entity, int (*map_grid)[MAX_MAP_SIZE][MAX_MAP_SIZE], bool* destroy_projectile) {
    *destroy_projectile = false;
    const double delta_x = entity->x_direction * entity->speed;
    const double delta_y = entity->y_direction * entity->speed;
    if (!isXCollision((SDL_FPoint) {.x = (float) entity->x, .y = (float) entity->y}, map_grid, delta_x, 5)) {
        entity->x += delta_x;
    } else {
        *destroy_projectile = true;
        return;
    }
    if (!isYCollision((SDL_FPoint) {.x = (float) entity->x, .y = (float) entity->y}, map_grid, delta_y, 5)) {
        entity->y += delta_y;
    } else {
        *destroy_projectile = true;
    }
}

static void setEntityData(Entity* entity) {
    FILE *file = fopen("game_data/entity_info/entity_info.csv", "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    char line[500];
    bool entity_set = false;
    while (fgets(line, sizeof(line), file) != NULL && !entity_set) {
        line[strcspn(line, "\r\n")] = '\0';
        const char *token = strtok(line, ",");
        int token_num = 0;
        while (token != NULL) {
            char* end_token;
            const int token_as_int = (int) strtol(token, &end_token, 10);
            if (token_num == 0 && entity->texture_id != token_as_int) {
                break;
            }
            switch (token_num) {
                case 1:
                    entity->scale = token_as_int;
                    break;
                case 2:
                    entity->height = token_as_int;
                    break;
                case 3:
                    entity->subtype_id = token_as_int;
                    break;
                case 4:
                    entity->health_amount = token_as_int;
                    break;
                case 5:
                    entity->weapon_index = token_as_int;
                    break;
                case 6:
                    entity->mag_amount = token_as_int;
                    break;
                case 7:
                    entity->ammo_type = token_as_int;
                    break;
                case 8:
                    entity->melee_range = token_as_int;
                    break;
                case 9:
                    entity->projectile_range = token_as_int;
                    break;
                case 10:
                    entity->melee_cool_down = token_as_int;
                    break;
                case 11:
                    entity->projectile_cool_down = token_as_int;
                    break;
                case 12:
                    entity->melee_damage = token_as_int;
                    break;
                case 13:
                    entity->projectile_damage = token_as_int;
                    break;
                case 14:
                    entity->projectile_index = token_as_int;
                    break;
                case 15:
                    entity->speed = token_as_int;
                    break;
                case 16:
                    entity->health = token_as_int;
                    entity_set = true;
                    break;
                default:
                    break;
            }
            token = strtok(NULL, ",");
            token_num++;
        }
    }
    fclose(file);
}