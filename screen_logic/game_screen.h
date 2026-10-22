#ifndef AFLICKEROFSTILLNESS_GAME_SCREEN_H
#define AFLICKEROFSTILLNESS_GAME_SCREEN_H
#include <SDL3/SDL_render.h>

#include "pause_screen.h"
#include "../program_data.h"
#include "../constants.h"

typedef struct {
    int num_map_textures;
    int num_entity_textures;
    SDL_Surface* map_surfaces[MAX_NUM_TEXTURES];
    SDL_Texture* map_textures[MAX_NUM_TEXTURES];
    SDL_Surface* entity_surfaces[MAX_NUM_TEXTURES];
    SDL_Texture* entity_textures[MAX_NUM_TEXTURES];
    SDL_Surface* sky_surface;
    SDL_Texture* sky_texture;
}GameScreen;

void initGameScreen(GameScreen** game_screen, const GameData* game_data);

void setGameScreenTextures(GameScreen* game_screen, SDL_Renderer* renderer);

void destroyGameScreen(GameScreen** game_screen);

/*
 * Painting
 */

int paintGameScreen(const GameScreen* game_screen, SDL_Renderer* renderer, GameData* game_data,
    const UserData* user_data);

static void drawSky(const GameScreen* game_screen, SDL_Renderer* renderer, const GameData* game_data);

static void drawCeilingLine(const GameScreen* game_screen, SDL_Renderer* renderer, const GameData* game_data,
    int vert_res, int ray_num, int start_y, double angle, double pre_adjustment);

static void drawFloorLine(const GameScreen* game_screen, SDL_Renderer* renderer, const GameData* game_data,
    int vert_res, int ray_num, float line_length, int start_y, double angle, double pre_adjustment);

static void drawWallLine(const GameScreen* game_screen, SDL_Renderer* renderer, const GameData* game_data,
    double angle, Ray ray, float line_length, int x, int y);

static void drawEntities(const GameScreen* game_screen, SDL_Renderer* renderer, GameData* game_data);

/*
 * Event Handling
 */

void gameScreenMousePressed(GameScreen* game_screen, SDL_FPoint mouse_pos);

int gameScreenMouseReleased(GameScreen* game_screen, SDL_FPoint mouse_pos);

void gameScreenMouseDragged(GameScreen* game_screen, SDL_FPoint mouse_pos, double prev_mouse_x,
    const GameData* game_data, const UserData* user_data);

void gameScreenMouseMoved(GameScreen* game_screen, SDL_FPoint mouse_pos, double prev_mouse_x,
    const GameData* game_data, const UserData* user_data);

void gameScreenKeyPressed(SDL_Keycode keycode, const GameData* game_data);

void gameScreenKeyReleased(SDL_Keycode keycode, const GameData* game_data);

int gameScreenKeyClicked(SDL_Keycode keycode);

/*
 * Helper functions
 */

static void setSurfacePixelColor(const SDL_Surface* surface, int x, int y, SDL_Color* out_color);

static void setEntityVectors(const GameData* game_data);

static void quickSortEntities(Entity* entities[MAX_NUM_ENTITIES], int low, int high);

static double getEntityAngle(const Entity* entity, double player_vect_x, double player_vect_y);

static int getTextureSide(double angle, Ray ray);

static SDL_Texture* getWallTexture(const GameScreen* game_screen, const GameData* game_data, double angle, Ray ray,
    int side);

static int getWallTextureIndexX(const GameData* game_data, double angle, Ray ray);

static int getWallTextureIndexY(const GameData* game_data, double angle, Ray ray);

static int getFloorX(const GameData* game_data, double angle, double post_adjustment);

static int getFloorY(const GameData* game_data, double angle, double post_adjustment);

static double getRayAngle(double ray_degree, double player_rotation);

/*
 * Surface Loading
 */

static int setMapSurfaces(GameScreen* game_screen);

static int setEntitySurfaces(GameScreen* game_screen);

static void setSkySurface(GameScreen* game_screen, int sky_texture_index);

#endif //AFLICKEROFSTILLNESS_GAME_SCREEN_H
