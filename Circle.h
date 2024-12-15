#pragma once
#include "Object.h"
#include "SDL2/SDL.h"


class Circle : public Object
{
public:
    Circle(Vector2D position, float rotation, float mass, float gravityMultiplier, float coefficient_of_restitution, int collision_layer, int* colliding_layers, int num_colliding_layers, float radius, SDL_Color color);

    void Draw(SDL_Window* win, Vector2D offset) override;

private:
    float radius;
    SDL_Color color;
};

void DrawCircle(SDL_Renderer *renderer, Vector2D pos, float radius, SDL_Color colour);