#ifndef AFLICKEROFSTILLNESS_WEAPON_H
#define AFLICKEROFSTILLNESS_WEAPON_H
#include <stdbool.h>

enum {
    WEAPON_MELEE, WEAPON_RANGED, WEAPON_AUTOMATIC, WEAPON_PROJECTILE
};

typedef struct {
    int entity_id;
    int weapon_id;
    int mag_count;
    int ammo_count;

    int type, damage, ammo_type, projectile_index, max_ammo_count, default_mag_count, max_mag_count, range,
    attack_cool_down, relead_speed;
}Weapon;

void initWeapon(Weapon** weapon, int weapon_id, int entity_id, int mag_count, int ammo_count);

void destroyWeapon(Weapon** weapon);

bool isPrimaryWeapon(const Weapon* weapon);

static int getEntityId(int weapon_id);

static void setWeaponInfo(Weapon* weapon);

#endif //AFLICKEROFSTILLNESS_WEAPON_H
