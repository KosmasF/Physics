#include "Object.h"

Object::Object(Vector2D position, float rotation, float mass, float gravityMultiplier, float coefficient_of_restitution, int collision_layer, int* colliding_layers, int num_colliding_layers)
{
    this->position = position;
    this->mass = mass;
    this->gravityMultiplier = gravityMultiplier;
    this->rotation = rotation;
    this->coefficient_of_restitution = coefficient_of_restitution;
    this->collision_layer = collision_layer;
    this->colliding_layers = (int*)malloc(num_colliding_layers * sizeof(int));
    memcpy(this->colliding_layers, colliding_layers, num_colliding_layers * sizeof(int));
    this->num_colliding_layers = num_colliding_layers;
}

Object::Object(Object &other)
{
    memcpy((void*)this, (void*)&other, sizeof(Object));
    this->colliding_layers = (int*)malloc(num_colliding_layers * sizeof(int));
    memcpy(this->colliding_layers, other.colliding_layers, num_colliding_layers * sizeof(int));
}

Object::~Object()
{
    free(colliding_layers);
}

void Object::Update(float deltaTime)
{
    Vector2D linearAcceleration = {0 , 0};

    //Calculate gravity
    linearAcceleration.y += GRAVITY * gravityMultiplier;

    //Frame log
    last_frame = {
        position,
        velocity,
        linearAcceleration,
        rotation,
        angularVelocity,
        0
    };

    //Apply linearAcceleration
    position +=  ((velocity * deltaTime) + (linearAcceleration * deltaTime * deltaTime * 0.5f));
    velocity += linearAcceleration * deltaTime;

    rotation += ((angularVelocity * deltaTime) + (0 * deltaTime * deltaTime * 0.5f));
    angularVelocity += 0 * deltaTime;
}