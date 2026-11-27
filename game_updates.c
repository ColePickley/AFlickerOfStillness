#include "game_updates.h"

#include <math.h>
#include "astar.h"
#include "file_handling/save_handling.h"

ScreenChange updateGame(GameData* game_data, SDL_Window* window) {
    updatePlayer(game_data);
    checkpointCheck(game_data, window);
    collisionCheck(game_data);
    //Check if player has died
    ScreenChange screen_change = NO_CHANGE;
    if (game_data->player->health <= 0) {
        screen_change = DEATH_SCREEN;
    }

    updateEntities(game_data);
    return screen_change;
}

void updatePlayer(GameData* game_data) {
    if (game_data->player->last_update_time == 0) {
        game_data->player->last_update_time = SDL_GetTicks();
    }
    movePlayer(game_data->player, &game_data->map_grid);
    bool is_moving = false;
    if (game_data->player->x_velocity != 0.0 || game_data->player->y_velocity != 0.0) {
        is_moving = true;
    }
    const uint64_t time_elapsed = SDL_GetTicks() - game_data->player->last_update_time;
    if (game_data->player->is_sprinting && is_moving) {
        game_data->player->sprint_time =
            max(0, game_data->player->sprint_time - (int) time_elapsed * SPRINT_DEPLETION_SPEED);
        game_data->player->last_sprint_time = SDL_GetTicks();
    } else if (!game_data->player->is_sprinting &&
            SPRINT_COOLDOWN_TIME < SDL_GetTicks() - game_data->player->last_sprint_time &&
            game_data->player->sprint_time < MAX_SPRINT_TIME) {
        game_data->player->sprint_time =
            min(MAX_SPRINT_TIME, game_data->player->sprint_time + (int) time_elapsed * SPRINT_REGEN_SPEED);
    }
    if (game_data->player->sprint_time == 0 && game_data->player->is_sprinting) {
        game_data->player->is_sprinting = false;
    }
    game_data->player->last_update_time = SDL_GetTicks();

    movePlayer(game_data->player, &game_data->map_grid);

    if (game_data->player->primary_selected && game_data->player->primary_weapon != NULL &&
            game_data->player->primary_weapon->type == WEAPON_AUTOMATIC && game_data->player->attacking) {
        playerRangedAttack(game_data->player, &game_data->entities, game_data->num_entities);
    }
}

void checkpointCheck(GameData* game_data, SDL_Window* window) {
    const SDL_Point player_pos = {
        .x = (int) game_data->player->x / GRID_SIZE,
        .y = (int) game_data->player->y / GRID_SIZE
    };
    if (game_data->map_grid[player_pos.y][player_pos.x] == CELL_CHECKPOINT &&
            !game_data->player->force_checkpoint_inactive) {
        bool new_checkpoint = true;
        if (game_data->current_checkpoint != NULL && game_data->current_checkpoint->x != player_pos.x &&
                game_data->current_checkpoint->y != player_pos.y) {
            for (int i = 0; i < game_data->num_hit_checkpoints; i++) {
                if (game_data->hit_checkpoints[i].x == player_pos.x &&
                        game_data->hit_checkpoints[i].y == player_pos.y) {
                    new_checkpoint = false;
                }
            }
        } else if (game_data->current_checkpoint != NULL) {
            game_data->player->checkpoint_active = true;
            new_checkpoint = false;
            if (!SDL_SetWindowRelativeMouseMode(window, false)) {
                SDL_Log("Could not enable relative mouse mode: %s", SDL_GetError());
            }
        }
        if (new_checkpoint) {
            writeSave(game_data);
            game_data->player->checkpoint_active = true;
            if (game_data->last_checkpoint != NULL) {
                destroyCheckpoint(&game_data->last_checkpoint);
            }
            if (game_data->current_checkpoint != NULL) {
                game_data->last_checkpoint = game_data->current_checkpoint;
            }
            game_data->current_checkpoint = dequeueCheckpoint(game_data->checkpoint_queue);
            game_data->hit_checkpoints[game_data->num_hit_checkpoints] = (SDL_Point) {
                .x = game_data->current_checkpoint->x,
                .y = game_data->current_checkpoint->y
            };
            game_data->num_hit_checkpoints++;
            if (!SDL_SetWindowRelativeMouseMode(window, false)) {
                SDL_Log("Could not enable relative mouse mode: %s", SDL_GetError());
            }
        }
    } else if (game_data->map_grid[player_pos.y][player_pos.x] != CELL_CHECKPOINT) {
        game_data->player->checkpoint_active = false;
        game_data->player->force_checkpoint_inactive = false;
        if (!SDL_SetWindowRelativeMouseMode(window, true)) {
            SDL_Log("Could not enable relative mouse mode: %s", SDL_GetError());
        }
    }
}

void collisionCheck(GameData* game_data) {
    /*
     * Check player-entity collisions
     */

    int i = 0;
    while (i < game_data->num_entities) {
        if (!playerEntityCollisionCheck(game_data, i)) {
            i++;
        }
    }

    /*
     * Check entity-entity collisions and dead enemies
     */

    i = 0;
    while (i < game_data->num_entities) {
        bool entity_removed = false;
        if (game_data->entities[i]->subtype_id == ENTITY_SUBTYPE_PROJECTILE) {
            for (int j = 0; j < game_data->num_entities; j++) {
                if (i == j) continue;
                entity_removed = entityEntityCollisionCheck(game_data, i, j);
                if (entity_removed) break;
            }
        } else if (getEntityType(game_data->entities[i]) == ENTITY_TYPE_ENEMY && game_data->entities[i]->health <= 0) {
            removeEntity(game_data, i);
            entity_removed = true;
        }
        if (!entity_removed) {
            i++;
        }
    }
}

void updateEntities(GameData* game_data) {
    /*
     * move entities
     */

    int i = 0;
    while (i < game_data->num_entities) {
        bool entity_removed = false;
        const int entity_type = getEntityType(game_data->entities[i]);
        if (entity_type == ENTITY_TYPE_ENEMY) {
            setEntityDirection(game_data->entities[i], game_data);
            moveEnemy(game_data->entities[i], &game_data->map_grid, game_data->entities, game_data->num_entities);
        } else if (entity_type == ENTITY_TYPE_PROJECTILE) {
            moveProjectile(game_data->entities[i], &game_data->map_grid, &entity_removed);
            if (entity_removed) {
                removeEntity(game_data, i);
            }
        }
        if (!entity_removed) {
            i++;
        }
    }

    /*
     * ranged enemy attack
     */

    for (int j = 0; j < game_data->num_entities; j++) {
        if (game_data->entities[j]->subtype_id != ENTITY_SUBTYPE_ENEMY_MELEE_AND_PROJECTILE &&
                game_data->entities[j]->subtype_id != ENTITY_SUBTYPE_ENEMY_PROJECTILE) {
            continue;
        }
        const uint64_t time_elapsed = SDL_GetTicks() - game_data->entities[j]->last_ranged_attack_time;
        if (time_elapsed > game_data->entities[j]->projectile_cool_down) {
            game_data->entities[j]->last_ranged_attack_time = SDL_GetTicks();
            const double vectX = game_data->entities[j]->x - game_data->player->x;
            const double vectY = (game_data->entities[j]->y - game_data->player->y) * -1.0;
            const double rotation = atan2(vectY, vectX) - PI / 2.0;
            const double x = game_data->entities[j]->x + sin(rotation) * 20;
            const double y = game_data->entities[j]->y + cos(rotation) * 20;
            Entity* entity;
            initDefaultEntity(&entity, game_data->entities[j]->projectile_index, x, y);
            entity->x_direction = sin(rotation);
            entity->y_direction = cos(rotation);
            addEntity(game_data, entity);
            game_data->entities[j]->last_ranged_attack_time = SDL_GetTicks();
        }
    }
}