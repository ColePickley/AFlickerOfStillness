#ifndef AFLICKEROFSTILLNESS_FUNCTIONS_H
#define AFLICKEROFSTILLNESS_FUNCTIONS_H
#include <stdint.h>
#include <SDL3/SDL.h>
#include "constants.h"

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))
#define len(array) (sizeof(array) / sizeof((array)[0]))

static double toRadians(const double degrees) {
    return degrees * 0.0174532925199433;
}

static double toDegrees(const double radians) {
    return radians * 57.2957795131;
}

static uint32_t x_state = 2463534242UL;

static uint32_t randInt(void) {
    uint32_t x = x_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x_state = x;
}

static void normalizePath(char* path) {
    if (path == NULL) return;

    for (int i = 0; path[i] != '\0'; i++) {
        if (path[i] == WRONG_SEP) {
            path[i] = TARGET_SEP;
        }
    }
}

static bool isXCollision(const SDL_FPoint pos, int (*map_grid)[MAX_MAP_SIZE][MAX_MAP_SIZE], const double delta_x,
        const int padding) {
    double adjusted_delta_x = delta_x;
    if (delta_x < 0) {
        adjusted_delta_x -= padding;
    } else if (delta_x > 0) {
        adjusted_delta_x += padding;
    }
    const int grid_x = (int) ((pos.x + adjusted_delta_x) / 64);
    const int grid_y = (int) (pos.y / 64);
    if ((*map_grid)[grid_y][grid_x] == 1) {
        return true;
    }
    if ((*map_grid)[grid_y][grid_x] == 2) {
        (*map_grid)[grid_y][grid_x] = -2;
    }
    return false;
}

static bool isYCollision(const SDL_FPoint pos, int (*map_grid)[MAX_MAP_SIZE][MAX_MAP_SIZE], const double delta_y,
        const int padding) {
    double adjusted_delta_y = delta_y;
    if (delta_y < 0) {
        adjusted_delta_y -= padding;
    } else if (delta_y > 0) {
        adjusted_delta_y += padding;
    }
    const int grid_x = (int) (pos.x / 64);
    const int grid_y = (int) ((pos.y + adjusted_delta_y) / 64);
    if ((*map_grid)[grid_y][grid_x] == 1) {
        return true;
    }
    if ((*map_grid)[grid_y][grid_x] == 2) {
        (*map_grid)[grid_y][grid_x] = -2;
    }
    return false;
}

#endif //AFLICKEROFSTILLNESS_FUNCTIONS_H
