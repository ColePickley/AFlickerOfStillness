#ifndef AFLICKEROFSTILLNESS_RAY_CALCULATOR_H
#define AFLICKEROFSTILLNESS_RAY_CALCULATOR_H
#include "program_data.h"

void setPlayerRays(const Player* player, int rows, int cols, const int map_grid[MAX_MAP_SIZE][MAX_MAP_SIZE]);

void setRay(double start_x, double start_y, double rotation, Ray* ray, int rows, int cols, const int map_grid[MAX_MAP_SIZE][MAX_MAP_SIZE]);

static double getAngle(const Ray* ray, double rotation);

static int getXDirection(double angle);

static int getYDirection(double angle);

static double getFirstDeltaX(double x, double angle);

static double getFirstDeltaY(double y, double angle);

static double getXStep(double angle);

static double getYStep(double angle);

static int getXStepGridX(int player_grid_x, int x_direction, int num_x_steps, int num_map_grid_cols);

static int getXStepGridY(double x_step_y, double angle, int num_map_grid_rows);

static int getYStepGridY(int player_grid_y, int y_direction, int num_y_steps, int num_map_grid_rows);

static int getYStepGridX(double y_step_x, double angle, int num_map_grid_cols);

#endif //AFLICKEROFSTILLNESS_RAY_CALCULATOR_H
