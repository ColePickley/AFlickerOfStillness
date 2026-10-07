#ifndef AFLICKEROFSTILLNESS_FUNCTIONS_H
#define AFLICKEROFSTILLNESS_FUNCTIONS_H

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))
#define len(array) (sizeof(array) / sizeof((array)[0]))

static double toRadians(const double degrees) {
    return degrees * 0.0174532925199433;
}

static double toDegrees(const double radians) {
    return radians * 57.2957795131;
}

#endif //AFLICKEROFSTILLNESS_FUNCTIONS_H
