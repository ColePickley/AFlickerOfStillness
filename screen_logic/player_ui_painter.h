#ifndef AFLICKEROFSTILLNESS_PLAYER_UI_PAINTER_H
#define AFLICKEROFSTILLNESS_PLAYER_UI_PAINTER_H
#include "SDL3/SDL_render.h"
#include "../program_data.h"

#define HOTBAR_HEIGHT 175

typedef struct {
    int num_weapon_textures;
    int hotbar_weapon1_texture_index;
    int hotbar_weapon2_texture_index;
    SDL_Surface* ammo_surface;
    SDL_Texture* ammo_texture;
    SDL_Surface* mag_surface;
    SDL_Texture* mag_texture;
    SDL_Surface* weapon_surfaces[MAX_NUM_WEAPON_TEXTURES];
    SDL_Texture* weapon_textures[MAX_NUM_WEAPON_TEXTURES];
    SDL_Surface* key_surfaces[MAX_NUM_KEYS];
    SDL_Texture* key_textures[MAX_NUM_KEYS];
    Label* hotbar_labels[5];
}PlayerUIPainter;

void initPlayerUIPainter(PlayerUIPainter** ui_painter, GameData* game_data);

void setPlayerUIPainterTextures(PlayerUIPainter* ui_painter, SDL_Renderer* renderer);

void destroyPlayerUIPainter(PlayerUIPainter** ui_painter);

void paintPlayerUI(PlayerUIPainter* ui_painter, SDL_Renderer* renderer, const Player* player,
    SDL_Texture* const entity_textures[MAX_NUM_MAP_TEXTURES]);

/*
 *----------------------------------------------------------------------------------------------------------------------
 * Helper functions
 *----------------------------------------------------------------------------------------------------------------------
 */

static void paintHotbar(const PlayerUIPainter* ui_painter, SDL_Renderer* renderer, const Player* player,
    SDL_Texture* const entity_textures[MAX_NUM_MAP_TEXTURES]);

static void paintKeys(PlayerUIPainter* ui_painter, SDL_Renderer* renderer, const Player* player,
    SDL_Texture* const entity_textures[MAX_NUM_MAP_TEXTURES]);

static void setPlayerUIPainterLabels(PlayerUIPainter* ui_painter);

static void setHotbarSurfaces(PlayerUIPainter* ui_painter);

static int setWeaponSurfaces(PlayerUIPainter* ui_painter);

#endif //AFLICKEROFSTILLNESS_PLAYER_UI_PAINTER_H
