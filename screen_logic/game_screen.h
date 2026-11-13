#ifndef AFLICKEROFSTILLNESS_GAME_SCREEN_H
#define AFLICKEROFSTILLNESS_GAME_SCREEN_H
#include <SDL3/SDL_render.h>

#include "pause_screen.h"
#include "player_ui_painter.h"
#include "../program_data.h"
#include "../constants.h"

#define MAX_WALKING_VIEW_BOB 10.0f
#define MAX_SPRINTING_VIEW_BOB 20.0f
#define WALKING_VIEW_BOB_SPEED 0.06f
#define SPRINTING_VIEW_BOB_SPEED 0.12f

typedef struct {
    bool bob_down;
    uint64_t last_paint_time;
    int num_map_textures;
    int num_entity_textures;
    float view_bob_state;
    SDL_Surface* map_surfaces[MAX_NUM_MAP_TEXTURES];
    SDL_Texture* map_textures[MAX_NUM_MAP_TEXTURES];
    SDL_Surface* entity_surfaces[MAX_NUM_MAP_TEXTURES];
    SDL_Texture* entity_textures[MAX_NUM_MAP_TEXTURES];
    SDL_Surface* sky_surface;
    SDL_Texture* sky_texture;
    PlayerUIPainter* ui_painter;
}GameScreen;

void initGameScreen(GameScreen** game_screen, GameData* game_data);

void setGameScreenTextures(GameScreen* game_screen, SDL_Renderer* renderer);

void destroyGameScreen(GameScreen** game_screen);

/*
 *----------------------------------------------------------------------------------------------------------------------
 * Painting
 *----------------------------------------------------------------------------------------------------------------------
 */

int paintGameScreen(GameScreen* game_screen, SDL_Renderer* renderer, GameData* game_data,
    const UserData* user_data);

static void drawSky(const GameScreen* game_screen, SDL_Renderer* renderer, const GameData* game_data);

static void setViewBobState(GameScreen* game_screen, const Player* player);

static void drawCeilingLine(const GameScreen* game_screen, SDL_Renderer* renderer, const GameData* game_data,
    int vert_res, int ray_num, int start_y, double angle, double pre_adjustment);

static void drawFloorLine(const GameScreen* game_screen, SDL_Renderer* renderer, const GameData* game_data,
    int vert_res, int ray_num, float line_length, int start_y, double angle, double pre_adjustment);

static void drawWallLine(const GameScreen* game_screen, SDL_Renderer* renderer, const GameData* game_data,
    double angle, Ray ray, float line_length, int x, int y);

static void drawEntities(const GameScreen* game_screen, SDL_Renderer* renderer, GameData* game_data);

/*
 *----------------------------------------------------------------------------------------------------------------------
 * Event Handling
 *----------------------------------------------------------------------------------------------------------------------
 */

void gameScreenMousePressed(GameScreen* game_screen, SDL_FPoint mouse_pos, const GameData* game_data);

int gameScreenMouseReleased(GameScreen* game_screen, SDL_FPoint mouse_pos, const GameData* game_data);

void gameScreenMouseClicked(GameScreen* game_screen, SDL_FPoint mouse_pos, GameData* game_data);

void gameScreenMouseDragged(GameScreen* game_screen, SDL_FPoint mouse_pos, const GameData* game_data,
    const UserData* user_data);

void gameScreenMouseMoved(GameScreen* game_screen, SDL_FPoint mouse_pos, const GameData* game_data,
    const UserData* user_data);

void gameScreenKeyPressed(SDL_Keycode keycode, const GameData* game_data);

void gameScreenKeyReleased(SDL_Keycode keycode, const GameData* game_data);

int gameScreenKeyClicked(SDL_Keycode keycode, const GameData* game_data);

/*
 *----------------------------------------------------------------------------------------------------------------------
 * Helper functions
 *----------------------------------------------------------------------------------------------------------------------
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
 *----------------------------------------------------------------------------------------------------------------------
 * Surface Loading
 *----------------------------------------------------------------------------------------------------------------------
 */

static int setMapSurfaces(GameScreen* game_screen);

static int setEntitySurfaces(GameScreen* game_screen);

static void setSkySurface(GameScreen* game_screen, int sky_texture_index);

#endif //AFLICKEROFSTILLNESS_GAME_SCREEN_H
