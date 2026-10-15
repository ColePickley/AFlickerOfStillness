#ifndef AFLICKEROFSTILLNESS_FUNCTIONS_H
#define AFLICKEROFSTILLNESS_FUNCTIONS_H
#include <stdint.h>

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


#endif //AFLICKEROFSTILLNESS_FUNCTIONS_H
