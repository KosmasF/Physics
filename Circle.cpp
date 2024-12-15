#include "Circle.h"

Circle::Circle(Vector2D position, float rotation, float mass, float gravityMultiplier, float coefficient_of_restitution, int collision_layer, int* colliding_layers, int num_colliding_layers, float radius, SDL_Color color) 
    : Object(position, rotation, mass, gravityMultiplier, coefficient_of_restitution, collision_layer, colliding_layers, num_colliding_layers)
{
    this->radius = radius;
    this->color = color;
}

void Circle::Draw(SDL_Window* win, Vector2D offset)
{
    SDL_Renderer* renderer = SDL_GetRenderer(win);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    SDL_Point* points = (SDL_Point*)malloc(radius * radius * 2 * 2 * sizeof(SDL_Point));
    int buffer = 0;
    for(int x = -radius; x < radius; x++)   
    {
        for(int y = -radius; y < radius; y++)
        {
            if(sqrt((x * x) + (y * y)) < radius)
            {
                points[buffer] = (SDL_Point){(int)position.x - x + offset.x, (int)position.y - y + offset.y};
                buffer++;
            }
        }
    }

    SDL_RenderDrawPoints( renderer, points, buffer );
    free(points);
}

void DrawCircle(SDL_Renderer *renderer, Vector2D pos, float radius, SDL_Color colour)
{
    SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, colour.a);

    SDL_Point* points = (SDL_Point*)malloc(radius * radius * 2 * 2 * sizeof(SDL_Point));
    int buffer = 0;
    for(int x = -radius; x < radius; x++)
    {
        for(int y = -radius; y < radius; y++)
        {
            if(sqrt((x * x) + (y * y)) < radius)
            {
                points[buffer] = (SDL_Point){(int)pos.x - x, (int)pos.y - y};
                buffer++;
            }
        }
    }

    SDL_RenderDrawPoints( renderer, points, buffer );
    free(points);
}
