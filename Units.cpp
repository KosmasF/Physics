#include "Units.h"

float dot(Vector2D a, Vector2D b)
{
    return (a.x * b.x) + (a.y * b.y);
}

float cross(Vector2D a, Vector2D b)
{
    return (a.x * b.y) - (a.y * b.x);
}

float normalize_angle(float angle)
{
    angle = fmod(angle, TAU); // Reduce angle within -2π and 2π

    // If angle is negative, bring it to [0, 2π)
    if (angle < 0) {
        angle += TAU;
    }

    return angle;
}

float angle_difference(float angle1, float angle2) {
    // Normalize each angle to [0, 2π)
    angle1 = normalize_angle(angle1);
    angle2 = normalize_angle(angle2);

    // Calculate the difference
    float diff = angle1 - angle2;

    // Normalize the difference to [-π, π)
    if (diff < -M_PIf) {
        diff += TAU;
    } else if (diff >= M_PIf) {
        diff -= TAU;
    }

    return diff;
}

// Function to check if two angles are pointing to each other
bool angles_point_to_each_other(float angle1, float angle2) {
    float angle_diff =  fabs(angle_difference(angle1, angle2));

    return angle_diff <= 90.f / RADIAN;
}

void TimeSetZero()
{
    start_time = clock() / (double)CLOCKS_PER_SEC;
}

void TimeElapsed(const char *message)
{
    double current_time = clock() / (double)CLOCKS_PER_SEC;
    printf("%s%lf\n", message, current_time - start_time);
    start_time = current_time;
}

float max(float a, float b) {
    return (a > b) ? a : b;
}
float min(float a, float b) {
    return (a < b) ? a : b;
}