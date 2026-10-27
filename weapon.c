#include "weapon.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void initWeapon(Weapon** weapon, const int weapon_id, const int entity_id, const int mag_count,
                const int ammo_count) {
    *weapon = malloc(sizeof(Weapon));
    if (*weapon == NULL) {
        perror("Error allocating memory for weapon");
        return;
    }
    (*weapon)->weapon_id = weapon_id;
    if (entity_id == -1) {
        (*weapon)->entity_id = getEntityId(weapon_id);
    } else {
        (*weapon)->entity_id = entity_id;
    }
    (*weapon)->mag_count = mag_count;
    (*weapon)->ammo_count = ammo_count;
    setWeaponInfo(*weapon);
    if (isPrimaryWeapon(*weapon) && mag_count == -1) {
        (*weapon)->mag_count = (*weapon)->default_mag_count;
        (*weapon)->ammo_count = (*weapon)->max_ammo_count;
    }
}

void destroyWeapon(Weapon** weapon) {
    free(*weapon);
    *weapon = NULL;
}

bool isPrimaryWeapon(const Weapon* weapon) {
    if (weapon->type == WEAPON_MELEE) {
        return false;
    }
    return true;
}

static int getEntityId(const int weapon_id) {
    FILE *file = fopen("game_data/entity_info/entity_info.csv", "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }
    char line[500];
    bool id_found = false;
    int entity_id = -1;
    while (fgets(line, sizeof(line), file) != NULL && !id_found) {
        line[strcspn(line, "\r\n")] = '\0';
        const char *token = strtok(line, ",");
        int token_num = 0;
        while (token != NULL) {
            char* end_token;
            const int token_as_int = (int) strtol(token, &end_token, 10);
            if (token_num == 0) {
                entity_id = token_as_int;
            } else if (token_num == 5) {
                if (token_as_int == weapon_id) {
                    id_found = true;
                }
                break;
            }
            token = strtok(NULL, ",");
            token_num++;
        }
    }
    fclose(file);
    return entity_id;
}

static void setWeaponInfo(Weapon* weapon) {
    FILE *file = fopen("game_data/weapon_info/weapon_info.csv", "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    char line[500];
    bool id_found = false;
    int weapon_id = -1;
    while (fgets(line, sizeof(line), file) != NULL && !id_found) {
        line[strcspn(line, "\r\n")] = '\0';
        const char *token = strtok(line, ",");
        int token_num = 0;
        while (token != NULL) {
            char* end_token;
            const int token_as_int = (int) strtol(token, &end_token, 10);
            switch (token_num) {
                case 0:
                    weapon_id = token_as_int;
                    break;
                case 1:
                    weapon->type = token_as_int;
                    break;
                case 2:
                    weapon->damage = token_as_int;
                    break;
                case 3:
                    weapon->ammo_type = token_as_int;
                    break;
                case 4:
                    weapon->projectile_index = token_as_int;
                    break;
                case 5:
                    weapon->max_ammo_count = token_as_int;
                    break;
                case 6:
                    weapon->default_mag_count = token_as_int;
                    break;
                case 7:
                    weapon->max_mag_count = token_as_int;
                    break;
                case 8:
                    weapon->range = token_as_int;
                    break;
                case 9:
                    weapon->attack_cool_down = token_as_int;
                    break;
                case 10:
                    weapon->relead_speed = token_as_int;
                    break;
                default:
                    break;
            }
            if (weapon_id != weapon->weapon_id) {
                break;
            }
            if (!id_found) {
                id_found = true;
            }
            token = strtok(NULL, ",");
            token_num++;
        }
    }
    fclose(file);
}