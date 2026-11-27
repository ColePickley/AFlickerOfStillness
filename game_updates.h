#ifndef AFLICKEROFSTILLNESS_GAME_UPDATES_H
#define AFLICKEROFSTILLNESS_GAME_UPDATES_H
#include "constants.h"
#include "program_data.h"

ScreenChange updateGame(GameData* game_data, SDL_Window* window);

void updatePlayer(GameData* game_data);

void checkpointCheck(GameData* game_data, SDL_Window* window);

void collisionCheck(GameData* game_data);

void updateEntities(GameData* game_data);

#endif //AFLICKEROFSTILLNESS_GAME_UPDATES_H
