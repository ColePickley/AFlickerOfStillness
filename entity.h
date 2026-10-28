#ifndef AFLICKEROFSTILLNESS_ENTITY_H
#define AFLICKEROFSTILLNESS_ENTITY_H
#include <stdbool.h>

#include "constants.h"
#include "SDL3/SDL_rect.h"

enum {
    ENTITY_TYPE_AESTHETIC, ENTITY_TYPE_COLLECTABLE, ENTITY_TYPE_PROJECTILE, ENTITY_TYPE_ENEMY
};

enum {
    ENTITY_SUBTYPE_AESTHETIC, ENTITY_SUBTYPE_COLLECTABLE_HEALTH, ENTITY_SUBTYPE_COLLECTABLE_WEAPON,
    ENTITY_SUBTYPE_COLLECTABLE_AMMO, ENTITY_SUBTYPE_COLLECTABLE_KEY, ENTITY_SUBTYPE_PROJECTILE,
    ENTITY_SUBTYPE_ENEMY_MELEE, ENTITY_SUBTYPE_ENEMY_PROJECTILE, ENTITY_SUBTYPE_ENEMY_MELEE_AND_PROJECTILE
};

typedef struct PathNode PathNode;

struct PathNode {
    SDL_Point pos;
    PathNode* next_node;
};

typedef struct {
    PathNode* start_node;
    uint16_t size;
}EntityPath;

typedef struct {
    // universal data
    double x, y, vect_x, vect_y, vect_mag;
    int subtype_id, texture_id, scale, height;
    uint64_t last_update_time;
    // type-specific data
    int health_amount; // collectable health
    int weapon_index, ammo_count, mag_count; // collectable weapon
    int mag_amount, ammo_type; // collectable ammo
    // enemies
    int melee_range, projectile_range, melee_cool_down, projectile_cool_down, melee_damage, projectile_damage,
        projectile_index, speed, health;
    bool in_range, in_scope;
    // movement
    EntityPath* path;
    double x_direction, y_direction;
}Entity;

void initDefaultEntity(Entity** entity, int texture_id, double x, double y);

void destroyEntity(Entity** entity);

int getEntityType(const Entity* entity);

void moveEnemy(Entity* entity, int (*map_grid)[MAX_MAP_SIZE][MAX_MAP_SIZE], Entity* entities[MAX_NUM_ENTITIES],
    int num_entities);

void moveProjectile(Entity* entity, int (*map_grid)[MAX_MAP_SIZE][MAX_MAP_SIZE], bool* destroy_projectile);

static void setEntityData(Entity* entity);

#endif //AFLICKEROFSTILLNESS_ENTITY_H
