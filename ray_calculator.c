#include "ray_calculator.h"

#include <math.h>
#include <stdbool.h>
#include "program_data.h"
#include "constants.h"
#include "functions.h"

void setPlayerRays(Player* player, const int rows, const int cols, const int map_grid[MAX_MAP_SIZE][MAX_MAP_SIZE]) {
    for (int i = 0; i < NUM_RAYS; i++) {
        setRay(player->x, player->y, player->rotation, &player->rays[i], rows, cols, map_grid);
    }
}

void setRay(const double start_x, const double start_y, const double rotation, Ray* ray, const int rows, const int cols,
        const int map_grid[MAX_MAP_SIZE][MAX_MAP_SIZE]) {
    const double angle = getAngle(ray, rotation);
    // the coordinates of the cell that the player/entity exists in
    const int player_grid_x = (int) (start_x / GRID_SIZE);
    const int player_grid_y = (int) (start_y / GRID_SIZE);
    // the direction that the ray is pointing (positive or negative x or y)
    const int x_direction = getXDirection(angle);
    const int y_direction = getYDirection(angle);
    // the distance from the player/entity to the first horizontal or vertical grid line that the ray will cross
    const double first_delta_x = getFirstDeltaX(start_x, angle);
    const double first_delta_y = getFirstDeltaY(start_y, angle);
    // the amount that y or x changes for every 1 unit you move in the x or y direction
    const double x_step = getXStep(angle);
    const double y_step = getYStep(angle);
    // the length of the ray for every x or y step
    const double x_step_length = sqrt(x_step * x_step + 1);
    const double y_step_length = sqrt(y_step * y_step + 1);
    /*
     * The player is generally not exactly on a grid line, so the first step that we take in both the x and y directions
     * is from the center of a cell to the respective grid line (depending on if you are doing an x or y step)
     */
    const double first_x_step = x_step * first_delta_x;
    const double first_y_step = y_step * first_delta_y;
    const double first_x_step_length = x_step_length * first_delta_x;
    const double first_y_step_length = y_step_length * first_delta_y;
    /*
     * We are doing, in concept, the same thing here that we did for the 4 lines above; however, each full step moves 64
     * units in the x or y direction, so instead of scaling things using our first step length, we scale them using our
     * standard step length of 1 cell (or 64 units)
     */
    const double scaled_x_step = x_step * (double) GRID_SIZE;
    const double scaled_y_step = y_step * (double) GRID_SIZE;
    const double scaled_x_step_length = x_step_length * (double) GRID_SIZE;
    const double scaled_y_step_length = y_step_length * (double) GRID_SIZE;
    // xSteps
    int num_x_steps = 0;
    double total_x_step_length = 0;
    double x_step_y = start_y;
    total_x_step_length += first_x_step_length;
    x_step_y += first_x_step;
    num_x_steps += 1;
    if (map_grid[getXStepGridY(x_step_y, angle, rows)][getXStepGridX(player_grid_x, x_direction, num_x_steps, cols)] <= 0) {
        bool is_collision = false;
        while (!is_collision) {
            total_x_step_length += scaled_x_step_length;
            x_step_y += scaled_x_step;
            num_x_steps += 1;
            if (map_grid[getXStepGridY(x_step_y, angle, rows)][getXStepGridX(player_grid_x, x_direction, num_x_steps, cols)] > 0) {
                is_collision = true;
            }
        }
    }
    // ySteps
    int num_y_steps = 0;
    double total_y_step_length = 0;
    double y_step_x = start_x;
    total_y_step_length += first_y_step_length;
    y_step_x += first_y_step;
    num_y_steps += 1;
    if (map_grid[getYStepGridY(player_grid_y, y_direction, num_y_steps, rows)][getYStepGridX(y_step_x, angle, cols)] <= 0) {
        bool is_collision = false;
        while (!is_collision) {
            total_y_step_length += scaled_y_step_length;
            y_step_x += scaled_y_step;
            num_y_steps += 1;
            if (map_grid[getYStepGridY(player_grid_y, y_direction, num_y_steps, rows)][getYStepGridX(y_step_x, angle, cols)] > 0) {
                is_collision = true;
            }
        }
    }
    // setting ray
    if (total_x_step_length < total_y_step_length) {
        ray->length = total_x_step_length;
        if (angle < 180.0) {
            ray->x = (player_grid_x + x_direction * num_x_steps) * (double) GRID_SIZE;
        } else {
            ray->x = (player_grid_x + x_direction * num_x_steps + 1.0) * (double) GRID_SIZE;
        }
        ray->y = x_step_y;
        ray->vert_collision = true;
    } else {
        ray->length = total_y_step_length;
        ray->x = y_step_x;
        if (angle >= 270 || angle < 90) {
            ray->y = (player_grid_y + y_direction * num_y_steps) * (double) GRID_SIZE;
        } else {
            ray->y = (player_grid_y + y_direction * num_y_steps + 1) * (double) GRID_SIZE;
        }
        ray->vert_collision = false;
    }
}

static double getAngle(const Ray* ray, const double rotation) {
    double angle = ray->degree + rotation;
    while (angle >= 360) {
        angle -= 360;
    }
    while (angle < 0) {
        angle += 360;
    }
    return angle;
}

static int getXDirection(const double angle) {
    if (angle < 180) {
        return 1;
    }
    return -1;
}

static int getYDirection(const double angle) {
    if (angle >= 270 || angle < 90) {
        return 1;
    }
    return -1;
}

static double getFirstDeltaX(const double x, const double angle) {
    if (angle < 180.0) {
        return GRID_SIZE - fmod(x, GRID_SIZE);
    }
    return fmod(x, GRID_SIZE);
}

static double getFirstDeltaY(const double y, const double angle) {
    if (angle >= 270.0 || angle < 90.0) {
        return GRID_SIZE - fmod(y, GRID_SIZE);
    }
    return fmod(y, GRID_SIZE);
}

static double getXStep(const double angle) {
    if (angle < 90.0) {
        return 1.0 / tan(toRadians(angle));
    }
    if (angle < 180.0) {
        return -1.0 * tan(toRadians(angle - 90));
    }
    if (angle < 270.0) {
        return -1.0 / tan(toRadians(angle - 180));
    }
    return tan(toRadians(angle - 270));
}

static double getYStep(const double angle) {
    double tan_angle = tan(toRadians(angle));
    if (tan_angle == 0) {
        tan_angle += 0.0001;
    }
    double tan_angle_90 = tan(toRadians(angle - 90));
    if (tan_angle_90 == 0) {
        tan_angle_90 += 0.0001;
    }

    if (angle < 90.0) {
        return tan_angle;
    }
    if (angle < 180.0) {
        return 1.0 / tan_angle_90;
    }
    if (angle < 270.0) {
        return -1.0 * tan_angle;
    }
    return -1.0 / tan_angle_90;
}

static int getXStepGridX(const int player_grid_x, const int x_direction, const int num_x_steps, const int num_map_grid_cols) {
    int grid_x = player_grid_x + x_direction * num_x_steps;
    grid_x = max(0, min(grid_x, num_map_grid_cols - 1));
    return grid_x;
}

static int getXStepGridY(const double x_step_y, const double angle, const int num_map_grid_rows) {
    int grid_y = (int) (x_step_y / GRID_SIZE);
    const double value = fmod(x_step_y / GRID_SIZE, 1);
    if ((fabs(value) < EPSILON || fabs(value - 1.0) < EPSILON) && 90.0 < angle && angle < 270.0) {
        grid_y -= 1;
    }
    grid_y = max(0, min(grid_y, num_map_grid_rows - 1));
    return grid_y;
}

static int getYStepGridY(const int player_grid_y, const int y_direction, const int num_y_steps, const int num_map_grid_rows) {
    int grid_y = player_grid_y + y_direction * num_y_steps;
    grid_y = max(0, min(grid_y, num_map_grid_rows - 1));
    return grid_y;
}

static int getYStepGridX(const double y_step_x, const double angle, const int num_map_grid_cols) {
    int grid_x = (int) (y_step_x / GRID_SIZE);
    const double value = fmod(y_step_x / (double) GRID_SIZE, 1);
    if ((fabs(value) < EPSILON || fabs(value - 1.0) < EPSILON) && 90.0 < angle && angle < 270.0) {
        grid_x -= 1;
    }
    grid_x = max(0, min(grid_x, num_map_grid_cols - 1));
    return grid_x;
}