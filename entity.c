#include "entity.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void initDefaultEntity(Entity* entity, const int type_index, const double x, const double y) {
    entity->type_index = type_index;
    entity->x = x;
    entity->y = y;
    entity->vect_x = 0.0;
    entity->vect_y = 0.0;
    entity->vect_mag = 0.0;
    entity->in_range = false;
    entity->in_scope = false;
    setEntityData(entity);
    entity->ammo_count = 0;
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
            if (token_num == 0 && entity->type_index != token_as_int) {
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
                    entity->entity_type = token_as_int;
                    break;
                case 4:
                    entity->health_amount = token_as_int;
                    break;
                case 5:
                    entity->weapon_index = token_as_int;
                    break;
                case 6:
                    entity->mag_count = token_as_int;
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