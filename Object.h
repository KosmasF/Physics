#pragma once
#include "Units.h"
#include "SDL2/SDL.h"

class Object
{
public:
    Object(Vector2D position, float rotation, float mass, float gravityMultiplier, float coefficient_of_restitution, int collision_layer, int* colliding_layers, int num_colliding_layers);
    Object(Object& other);
    ~Object();

    void Update(float deltaTime);
    virtual void Draw(SDL_Window*, Vector2D) {}
    virtual CollisionsData CheckForCollision(Object**, int, float, int = 0) {return CollisionsData();}

    FrameData last_frame;

protected:
    Vector2D position;
    Vector2D CenterOfMass;
    //Vector2D acceleration;
    Vector2D velocity = {0, 0};

    float mass;
    float momentOfInertia;
    float rotation;
    float angularVelocity = 0;
    //float angularAcceleration;
    float gravityMultiplier;

    float coefficient_of_restitution;

    // COLISION DATA
    int collision_layer;
    int* colliding_layers;
    int num_colliding_layers;
};