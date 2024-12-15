#pragma once
#include "math.h"

#define GRAVITY 9.18f
#define COLLISION_SEARCH_DEPTH 20
#define RADIAN (180.f / M_PIf) //~53
#define MAX_COL_REC_FOR_RESING 20
#define NULL_VEC {0,0}
#define TAU (2 * M_PIf)

#define RED {0xff, 0x00, 0x00, 0xff}
#define BLUE {0x00, 0x00, 0xff, 0xff}
#define GREEN {0x00, 0xff, 0x00, 0xff}

#define HEIGHT 1080 / 2
#define WIDTH 1920 / 2

float max(float a, float b);
float min(float a, float b);


struct Vector2D
{
    float x;
    float y;

    Vector2D operator*(const Vector2D& other)
    {
        return {x * other.x, y * other.y};
    }

    Vector2D operator/(const Vector2D& other)
    {
        return {x / other.x, y / other.y};
    }

    Vector2D operator*=(const Vector2D& other)
    {
        x *= other.x;
        y *= other.y;
        return *this;
    }

    Vector2D operator+=(const Vector2D& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vector2D operator+(const Vector2D& other)
    {
        return {x + other.x, y + other.y};
    }

    Vector2D operator-=(const Vector2D& other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }


    Vector2D operator-(const Vector2D& other)
    {
        return {x - other.x, y - other.y};
    }

    Vector2D operator*(const float scalar)
    {
        return {x * scalar, y * scalar};
    }

    Vector2D operator/(const float scalar)
    {
        return {x / scalar, y / scalar};
    }

    float angle()
    {
        return atan2(y,x);
    }

    float len()
    {
        return sqrt((x*x) + (y*y));
    }

    Vector2D perpendicular() {
        return {-y, x};
    }

    Vector2D parallel()
    {
        return {-x, -y};
    }

    Vector2D rotate(float angle)
    {
        double absolute_value = sqrtf((x * x) + (y * y));
        double point_angle = atan2f(y , x);

        Vector2D point_rotated = {
            (float)(cos(point_angle + angle) * absolute_value), //For some reason it is subtraction, I dont know why??
            (float)(sin(point_angle + angle) * absolute_value)
        };

        return point_rotated;
    }

    //float operator*(const Vector2D& other)//Dot product
    //{
        
    //}
};

float dot(Vector2D a, Vector2D b);

float cross(Vector2D a, Vector2D b);

template<typename Type>
bool array_contains(Type* array, int num, Type obj);

struct FrameData
{
    Vector2D startingPos;
    Vector2D startingVel;
    Vector2D linearAccel;
    float startingRot;
    float startingAngVel;
    float angularAccel;
};

template <typename Type>
inline bool array_contains(Type *array, int num, Type obj)
{
    for(int i = 0; i < num; i++)
    {
        if(array[i] == obj)
            return true;
    }
    return false;
}

struct CollisionData
{
    int collision_layer_1;
    int collision_layer_2;
};

struct CollisionsData
{
    int num;
    CollisionData* data;
};

float normalize_angle(float angle);

// angle1 - angle2
float angle_difference(float angle1, float angle2);

// Function to check if two angles are pointing to each other
bool angles_point_to_each_other(float angle1, float angle2);