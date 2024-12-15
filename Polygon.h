#pragma once
#include "Object.h"
#include "SDL2/SDL.h"
#include "Circle.h"
#include "Units.h"
#include "../Discrete-Fourier-Transform/Map.h"

class Polygon: public Object
{
public:
    Polygon(Vector2D position, float rotation, float mass, float gravityMultiplier, float coefficient_of_restitution, int collision_layer, int* colliding_layers, int num_colliding_layers, Vector2D* points, int num_points, SDL_Color color);
    Polygon(Polygon& other);
    ~Polygon();

    void Draw(SDL_Window* win, Vector2D offset) override;
    void DrawInArray(float* array, int height, int width, int array_height, int array_width);
    CollisionsData CheckForCollision(Object** objects, int num_opbjects, float deltaTime, int n = 0) override;//func is recursive

    void ApplyForce(Vector2D pos, Vector2D force, float deltaTime);
    void ApplyAccel(Vector2D pos, Vector2D force, float deltaTime);

    Vector2D* RotateAndTranslate();
    Vector2D ReturnPosition() {return position;}
    int ReturnCollisionLayer() {return collision_layer;}
    float ReturnRotation() {return rotation;}

private:
    Vector2D* points;
    int num_points;
    SDL_Color color;

    Vector2D GetCenterOfMass();
    float GetArea();
    float GetMomentOfInertia();


    bool IsPointInPolygon(Vector2D* poly_points, int num, Vector2D point);
    bool IsPolygonColliding(Vector2D* poly1, int num1, Vector2D* poly2, int num2, int* pointOfCollision);
    uint NumPointsColliding(Vector2D* poly1, int num1, Vector2D* poly2, int num2);

    Vector2D* PointsColliding(Vector2D* poly1, int num1, Vector2D* poly2, int num2);
    Vector2D* PointInWichLine(Vector2D* poly, int num, Vector2D point);

};