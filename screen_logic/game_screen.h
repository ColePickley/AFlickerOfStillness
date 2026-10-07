#ifndef AFLICKEROFSTILLNESS_GAME_SCREEN_H
#define AFLICKEROFSTILLNESS_GAME_SCREEN_H
#include <SDL3/SDL_render.h>
#include "../program_data.h"
#include "../constants.h"

typedef struct {
    SDL_Texture* texture[TEXTURE_RESOLUTION];
    SDL_Surface* surface[TEXTURE_RESOLUTION];
}SlicedTexture;

typedef struct {
    bool paint_game_screen;
    bool paint_pause_screen;
    int num_map_textures;
    int num_entity_textures;
    SlicedTexture* map_textures[MAX_NUM_TEXTURES];
    SlicedTexture* entity_textures[MAX_NUM_TEXTURES];
}GameScreen;

void initGameScreen(GameScreen* game_screen, SDL_Renderer* renderer);

void destroyGameScreen(GameScreen* game_screen);

void paintGameScreen(const GameScreen* game_screen, SDL_Renderer* renderer, GameData* game_data, const UserData* user_data);

static void draw_ceiling_line(const GameScreen* game_screen, SDL_Renderer* renderer, const GameData* game_data,
    int vert_res, int ray_num, int start_y, double angle, double pre_adjustment);

static void draw_floor_line(const GameScreen* game_screen, SDL_Renderer* renderer, const GameData* game_data,
    int vert_res, int ray_num, float line_length, int start_y, double angle, double pre_adjustment);

static void draw_wall_line(const GameScreen* game_screen, SDL_Renderer* renderer, const GameData* game_data,
    double angle, Ray ray, float line_length, int x, int y);

static void draw_entities(const GameScreen* game_screen, SDL_Renderer* renderer, GameData* game_data);

/*
 * Helper functions
 */

static void set_surface_pixel_color(const SDL_Surface* surface, int x, int y, SDL_Color* out_color);

static void set_entity_vectors(const GameData* game_data);

static void quick_sort_entities(Entity* entities[MAX_NUM_ENTITIES], int low, int high);

static double getEntityAngle(const Entity* entity, double player_vect_x, double player_vect_y);

static int get_texture_side(double angle, Ray ray);

static SlicedTexture* get_wall_texture(const GameScreen* game_screen, const GameData* game_data, double angle, Ray ray,
    int side);

static int get_wall_texture_index_x(const GameData* game_data, double angle, Ray ray);

static int get_wall_texture_index_y(const GameData* game_data, double angle, Ray ray);

static int get_floor_x(const GameData* game_data, double angle, double post_adjustment);

static int get_floor_y(const GameData* game_data, double angle, double post_adjustment);

static double get_ray_angle(double ray_degree, double player_rotation);

static int set_map_textures(GameScreen* game_screen, SDL_Renderer* renderer);

static int set_entity_textures(GameScreen* game_screen, SDL_Renderer* renderer);

#endif //AFLICKEROFSTILLNESS_GAME_SCREEN_H
