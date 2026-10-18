#ifndef AFLICKEROFSTILLNESS_ENTITY_H
#define AFLICKEROFSTILLNESS_ENTITY_H
#include <stdbool.h>

typedef struct {
    //universal data
    double x, y, vect_x, vect_y, vect_mag;
    int type_index, scale, height, entity_type;
    // type-specific data
    int health_amount, weapon_index, ammo_count, mag_count, ammo_type, melee_range, projectile_range,
        melee_cool_down, projectile_cool_down, melee_damage, projectile_damage, projectile_index, speed, health;
    bool in_range, in_scope;
}Entity;

void initDefaultEntity(Entity* entity, int type_index, double x, double y);

static void setEntityData(Entity* entity);

#endif //AFLICKEROFSTILLNESS_ENTITY_H
