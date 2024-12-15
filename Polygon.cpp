#include "Polygon.h"

Polygon::Polygon(Vector2D position, float rotation, float mass, float gravityMultiplier, float coefficient_of_restitution, int collision_layer, int* colliding_layers, int num_colliding_layers, Vector2D* points, int num_points, SDL_Color color)
: Object(position, rotation, mass, gravityMultiplier, coefficient_of_restitution,   collision_layer, colliding_layers, num_colliding_layers)
{
    this->num_points = num_points;
    this->points = (Vector2D*)malloc(num_points * sizeof(Vector2D));
    memcpy(this->points, points, num_points * sizeof(Vector2D));
    this->color = color;

    CenterOfMass = GetCenterOfMass();
    momentOfInertia = GetMomentOfInertia();
}

Polygon::Polygon(Polygon &other)
: Object(other)
{
    memcpy((void*)(((Object*)this) + 1), (void*)(((Object*)&other) + 1), sizeof(Polygon) - sizeof(Object));//Object copy cobst was called before , STUPID!!!
    this->points = (Vector2D*)malloc(num_points * sizeof(Vector2D));
    memcpy(this->points, other.points, num_points * sizeof(Vector2D));
}

Polygon::~Polygon()
{
    free(points);
}

void Polygon::Draw(SDL_Window *win)
{
    SDL_Renderer* renderer = SDL_GetRenderer(win);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);


    Vector2D* points_translated = RotateAndTranslate();

    float max_x, max_y, min_x, min_y;
    max_x = max_y = 0;
    min_y = min_x = INFINITY;
    for(int i = 0; i < num_points; i++)
    {
        if(points_translated[i].x > max_x)
            max_x = points_translated[i].x;

        if(points_translated[i].x < min_x)
            min_x = points_translated[i].x;

        if(points_translated[i].y > max_y)
            max_y = points_translated[i].y;

        if(points_translated[i].y < min_y)
            min_y = points_translated[i].y;
    }
    for(int x = min_x; x < max_x; x++)
    {
        for(int y = min_y; y < max_y; y++)
        {                    
            if(IsPointInPolygon(points_translated, num_points, {(float)x, (float)y}))
            {

                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }

    free(points_translated);

    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderDrawPoint(renderer, CenterOfMass.x + position.x, CenterOfMass.y + position.y);

    //rotation+=0.001f;
}

void Polygon::DrawInArray(float *array, int height, int width, int array_height, int array_width)
{
    Vector2D* points_translated = RotateAndTranslate();
    for(int i = 0; i < num_points; i++)
    {
        points_translated[i].x = map(points_translated[i].x, 0, width, 0, array_width);
        points_translated[i].y = map(points_translated[i].y, 0, height, 0, array_height);
    }

    float max_x, max_y, min_x, min_y;
    max_x = max_y = 0;
    min_y = min_x = INFINITY;
    for(int i = 0; i < num_points; i++)
    {
        if(points_translated[i].x > max_x)
            max_x = points_translated[i].x;

        if(points_translated[i].x < min_x)
            min_x = points_translated[i].x;

        if(points_translated[i].y > max_y)
            max_y = points_translated[i].y;

        if(points_translated[i].y < min_y)
            min_y = points_translated[i].y;
    }
    for(int x = min_x; x < max_x; x++)
    {
        for(int y = min_y; y < max_y; y++)
        {                    
            if(IsPointInPolygon(points_translated, num_points, {(float)x, (float)y}))
            {

                array[x + (y * array_width)] = 1;
            }
        }
    }

    free(points_translated);
}

bool Polygon::IsPointInPolygon(Vector2D* poly_points, int num, Vector2D point)
{
    int intersections = 0;

    for(int i = 0; i < num - 1; i++)
    {
        bool p0 = poly_points[i].y < point.y;
        bool p1 = poly_points[i + 1].y < point.y;

        bool passing = p0 ^ p1;

        if(passing)
        {
            float tan_angle = (poly_points[i].y - poly_points[i + 1].y) / (poly_points[i].x - poly_points[i + 1].x);
            float rel_intersection_x = (point.y - poly_points[i].y) / tan_angle;
            float intersection_x = poly_points[i].x + rel_intersection_x;

            if(intersection_x > point.x)
                intersections++;
        }
    }

    {
        bool p0 = poly_points[num - 1].y < point.y;
        bool p1 = poly_points[0].y < point.y;

        bool passing = p0 ^ p1;

        if(passing)
        {
            float tan_angle = (poly_points[num - 1].y - poly_points[0].y) / (poly_points[num - 1].x - poly_points[0].x);
            float rel_intersection_x = (point.y - poly_points[num - 1].y) / tan_angle;
            float intersection_x = poly_points[num - 1].x + rel_intersection_x;

            if(intersection_x > point.x)
                intersections++;
        }
    }

    return !(intersections % 2 == 0);
}

Vector2D *Polygon::PointsColliding(Vector2D *poly1, int num1, Vector2D *poly2, int num2)
{
    Vector2D* points = (Vector2D*)malloc(NumPointsColliding(poly1, num1, poly2, num2) * sizeof(Vector2D));
    uint num = 0;
    for(int i = 0; i < num1; i++)
    {
        if(IsPointInPolygon(poly2, num2, poly1[i]))
        {
            points[num] = poly1[i];
            num++;
        }
    }

    return points;
}

Vector2D *Polygon::PointInWichLine(Vector2D *poly, int num, Vector2D point)
{
    float min_len = INFINITY;
    int idx;

    for(int i = 0; i < num; i++)
    {
        int next = (i + 1) % num;

        float pointAngle = atan2(point.y - poly[i].y, point.x - poly[i].x);
        float len = sqrt(pow(point.x - poly[i].x, 2) + pow(point.y - poly[i].y, 2));
        float sideAngle = atan2(poly[next].y - poly[i].y, poly[next].x - poly[i].x);

        float size = abs(sin(sideAngle - pointAngle) * len);
        
        if(size < min_len)
        {
            min_len = size;
            idx = i;
        }   
    }

    float pointAngle = atan2(point.y - poly[num - 1].y, point.x - poly[num - 1].x);
    float len = sqrt(pow(point.x - poly[num - 1].x, 2) + pow(point.y - poly[num - 1].y, 2));
    float sideAngle = atan2(poly[0].y - poly[num - 1].y, poly[0].x - poly[num - 1].x);

    float size = abs(sin(sideAngle - pointAngle) * len);
    
    if(size < min_len)
    {
        min_len = size;
        idx = num - 1;
    }   

    Vector2D* points = (Vector2D*)malloc(2 * sizeof(Vector2D));
    points[0] = poly[idx];
    points[1] = poly[idx != num - 1 ? idx + 1: 0];

    return points;
}

bool Polygon::IsPolygonColliding(Vector2D *poly1, int num1, Vector2D *poly2, int num2, int *pointOfCollison)
{
    for(int i = 0; i < num1; i++)
    {
        if(IsPointInPolygon(poly2, num2, poly1[i]))
        {
            *pointOfCollison = i;
            return true;
        }
    }

    return false;
}

uint Polygon::NumPointsColliding(Vector2D* poly1, int num1, Vector2D *poly2, int num2)
{
    uint num = 0;
    for(int i = 0; i < num1; i++)
    {
        if(IsPointInPolygon(poly2, num2, poly1[i]))
        {
            num++;
        }
    }

    return num;
}

CollisionsData Polygon::CheckForCollision(Object **objects, int num_opbjects, float deltaTime, int n)
{
    CollisionsData data = {0, nullptr};

    Vector2D* transformedPoints = RotateAndTranslate();
    int pointOfCollision_id;
    for(int i = 0; i < num_opbjects; i++)
    {
        if(objects[i] != this && array_contains<int>(colliding_layers, num_colliding_layers, ((Polygon*)objects[i])->collision_layer))
        for(int point = 0; point < num_points; point++)
        {
            uint num_collision_points;
            Vector2D* last_collision_this_points = RotateAndTranslate();
            Vector2D* last_collision_other_points = ((Polygon*)objects[i])->RotateAndTranslate();
            Vector2D* other_points = ((Polygon*)objects[i])->RotateAndTranslate();
            bool interpenetrating = (IsPointInPolygon(other_points, ((Polygon*)objects[i])->num_points, transformedPoints[point]));
            if(interpenetrating)
            {
                pointOfCollision_id = point;
                num_collision_points = NumPointsColliding(transformedPoints, num_points, other_points, ((Polygon*)objects[i])->num_points);
            }

            // ccc
            Polygon* object = ((Polygon*)objects[i]);
            float angle = atan2f((position + CenterOfMass).y - (object->position + object->CenterOfMass).y, (position + CenterOfMass).x - (object->position + object->CenterOfMass).x);
            float angle_diff =  fabs(angle_difference(velocity.angle(), angle));
            float angle_diff_other = fabs(angle_difference(object->velocity.angle(), angle));

            bool vel_colliding = angle_diff <= 90.f / RADIAN;
            bool vel_colliding_other = angle_diff_other <= 90.f / RADIAN;
            bool vel_is_bigger = velocity.len() > object->velocity.len();

            bool colliding = ((vel_colliding && vel_colliding_other) || vel_is_bigger) || 
            (vel_colliding && (!vel_colliding_other) && vel_is_bigger) ||
            (!vel_colliding && vel_colliding_other && (!vel_is_bigger));
            // if(colliding)
            //     printf("collidong!!\n");
            // else
            //     printf("WQAY\n");
            // if(interpenetrating)
            //     printf("niint\n");


            free(other_points);
            if(colliding && interpenetrating)
            // if(abs(((Polygon*)objects[i])->velocity.angle() - velocity.angle()) > M_PIf / 2 || velocity.angle() == 0 || ((Polygon*)objects[i])->velocity.angle() == 0)//90 deg
            // {
                // if(((Polygon*)objects[i])->velocity.len() < velocity.len() && interpenetrating)
                {
                    if(data.num == 0)
                    {
                        data.data = (CollisionData*)malloc(1 * sizeof(CollisionData));
                        data.data[0].collision_layer_1 = collision_layer;
                        data.data[0].collision_layer_2 = ((Polygon*)objects[i])->collision_layer;
                    }
                    else{
                        CollisionData* new_data = (CollisionData*)malloc((data.num + 1) * sizeof(CollisionData));
                        memcpy(new_data, data.data, sizeof(CollisionData) * data.num);
                        new_data[data.num - 1].collision_layer_1 = collision_layer;
                        new_data[data.num - 1].collision_layer_2 = ((Polygon*)objects[i])->collision_layer;
                        free(data.data);
                        data.data = new_data;
                    }
                    data.num++;
                    // printf("collision id happening!!!\n");
                    //Resolve Collision

                    Polygon* collinding_object = ((Polygon*)objects[i]);
                    float totalDeltaTime = deltaTime;

                    //Undo deltTime changes
                    for(int i = 0; i < COLLISION_SEARCH_DEPTH; i++)
                    {
                        position -= ((last_frame.startingVel * deltaTime) + (last_frame.linearAccel * deltaTime * deltaTime * 0.5f));
                        velocity -= last_frame.linearAccel * deltaTime;
                        rotation -= (last_frame.startingAngVel * deltaTime) + (last_frame.angularAccel * deltaTime * deltaTime * 0.5f);
                        angularVelocity -= last_frame.angularAccel * deltaTime;

                        collinding_object->position -= ((collinding_object->last_frame.startingVel * deltaTime) + (collinding_object->last_frame.linearAccel * deltaTime * deltaTime * 0.5f));
                        collinding_object->velocity -= collinding_object->last_frame.linearAccel * deltaTime;
                        collinding_object->rotation -= (collinding_object->last_frame.startingAngVel * deltaTime) + (collinding_object->last_frame.angularAccel * deltaTime * deltaTime * 0.5f);
                        collinding_object->angularVelocity -= collinding_object->last_frame.angularAccel * deltaTime;

                        deltaTime -= deltaTime;

                        position += ((last_frame.startingVel * deltaTime) + (last_frame.linearAccel * deltaTime * deltaTime * 0.5f));
                        velocity += last_frame.linearAccel * deltaTime;
                        rotation += (last_frame.startingAngVel * deltaTime) + (last_frame.angularAccel * deltaTime * deltaTime * 0.5f);
                        angularVelocity += last_frame.angularAccel * deltaTime;

                        collinding_object->position += ((collinding_object->last_frame.startingVel * deltaTime) + (collinding_object->last_frame.linearAccel * deltaTime * deltaTime * 0.5f));
                        collinding_object->velocity += collinding_object->last_frame.linearAccel * deltaTime;
                        collinding_object->rotation += (collinding_object->last_frame.startingAngVel * deltaTime) + (collinding_object->last_frame.angularAccel * deltaTime * deltaTime * 0.5f);
                        collinding_object->angularVelocity += collinding_object->last_frame.angularAccel * deltaTime;

                        Vector2D* pointsNorm = RotateAndTranslate();
                        Vector2D* collinding_object_pointsNorm = collinding_object->RotateAndTranslate();

                        if(IsPolygonColliding(pointsNorm, num_points, collinding_object_pointsNorm, collinding_object->num_points, &pointOfCollision_id))
                        {
                            num_collision_points = NumPointsColliding(pointsNorm, num_points, collinding_object_pointsNorm, collinding_object->num_points);
                                free(last_collision_this_points);
                                free(last_collision_other_points);
                            last_collision_this_points = pointsNorm;
                            last_collision_other_points = collinding_object_pointsNorm;
                            deltaTime-=deltaTime/2;
                        }
                        else
                            deltaTime+=deltaTime/2;
                    }


                    //we have found the time of colllision!!!
                    Vector2D* this_points = RotateAndTranslate();
                    Vector2D* collinding_object_points = collinding_object->RotateAndTranslate();

                   

                    float avg_coef_of_restitution = (coefficient_of_restitution + collinding_object->coefficient_of_restitution) / 2;
                    // bool resting = false;
                    // float grav = GRAVITY * deltaTime * gravityMultiplier;
                    // if(fabs(velocity.y) <= GRAVITY * deltaTime)
                    // {
                    //     avg_coef_of_restitution = 1.f;//FIX RESTING!!!!
                    //     printf("Resting\n");
                    //     resting = true;
                    // }
                    //We are here because a vertex of our polygon is inside the other, it is an edge/vertex collision



                    // num_collision_points = NumPointsColliding(this_points, num_points, collinding_object_points, collinding_object->num_points);
                    // Fix edge normal!!!
                    Vector2D pointOfCollision;
                    if(num_collision_points == 1)//Vertex
                    {
                        pointOfCollision = this_points[pointOfCollision_id];
                    }
                    else if(num_collision_points == 2)//Edge
                    {
                        Vector2D* colliding_points = PointsColliding(last_collision_this_points, num_points, last_collision_other_points, collinding_object->num_points);
                        pointOfCollision = colliding_points[0] + ((colliding_points[1] - colliding_points[0]) / 2.f);
                    }   
                    else
                    {
                        printf("Invalid collision!\n");
                    }

                    Vector2D* collisionPoints = PointInWichLine(collinding_object_points, collinding_object->num_points, pointOfCollision);
                    float angle = atan2(collisionPoints[1].y - collisionPoints[0].y, collisionPoints[1].x - collisionPoints[0].x);
                    Vector2D unitVector = {
                        cosf(angle) ,
                        sinf(angle)
                    };
                    Vector2D normal = unitVector.perpendicular() ;//* (!vel_is_bigger ? -1 : 1);
                        //normal = normal.parallel();
                    free(collisionPoints);

                    Vector2D AP = pointOfCollision - (CenterOfMass + position);
                    //AP = AP.parallel();
                    Vector2D BP = pointOfCollision - (collinding_object->CenterOfMass + collinding_object->position);


                    Vector2D relativeVelocity = (velocity + (AP.perpendicular() * angularVelocity)) - (collinding_object->velocity + (BP.perpendicular() * collinding_object->angularVelocity));


                    float J = 
                        dot
                        (
                            (
                                relativeVelocity
                            )
                            *
                            (
                                -(1 + avg_coef_of_restitution)
                            )
                            ,
                            normal
                        );

                        J/=
                        (

                            (1 / mass) 
                            + 
                            (1/collinding_object->mass)

                            +

                            (
                                pow
                                (
                                    dot(AP.perpendicular(), normal), 2
                                )
                                /
                                momentOfInertia
                            )

                            +

                            (
                                pow
                                (
                                    dot(BP.perpendicular(), normal), 2
                                )
                                /
                                collinding_object->momentOfInertia
                            )
                        )

                        ;

                    velocity = velocity + ( normal * (J / mass)) ;
                    collinding_object->velocity = collinding_object->velocity - (normal * (J / collinding_object->mass));

                    angularVelocity = angularVelocity + (dot(AP.perpendicular() , normal * J) / momentOfInertia);
                    collinding_object->angularVelocity = collinding_object->angularVelocity - (dot(BP.perpendicular() , normal * J) / collinding_object->momentOfInertia);



                    // I must fix this !!!

                    float residualDeltaTime = totalDeltaTime - deltaTime;
                    // position += ((velocity * residualDeltaTime) + (last_frame.linearAccel * residualDeltaTime * residualDeltaTime * 0.5f));
                    // velocity += last_frame.linearAccel * residualDeltaTime;
                    // rotation += (last_frame.startingRot * residualDeltaTime) + (last_frame.angularAccel * residualDeltaTime * residualDeltaTime * 0.5f);
                    // angularVelocity += last_frame.angularAccel * residualDeltaTime;

                    // collinding_object->position += ((collinding_object->velocity * residualDeltaTime) + (collinding_object->last_frame.linearAccel * residualDeltaTime * residualDeltaTime * 0.5f));
                    // collinding_object->velocity += collinding_object->last_frame.linearAccel * residualDeltaTime;
                    // collinding_object->rotation += (collinding_object->last_frame.startingRot * residualDeltaTime) + (collinding_object->last_frame.angularAccel * residualDeltaTime * residualDeltaTime * 0.5f);
                    // collinding_object->angularVelocity += collinding_object->last_frame.angularAccel * residualDeltaTime;               

                    // haha I fixed  this!!

                    // Update(residualDeltaTime);
                    // collinding_object->Update(residualDeltaTime);


                    // ----------MUST FIX RESIDUAL DELTA TIME----------------
                    //                  now it is lost

                    // Do it again!!
                    // if(residualDeltaTime > FLT_EPSILON && n < MAX_COL_REC_FOR_RESING)
                    //     CheckForCollision(objects, num_opbjects, residualDeltaTime, renderer, n + 1);
                    // else
                    // {
                    //     velocity = NULL_VEC;
                    //     collinding_object->velocity = NULL_VEC;
                    //     angularVelocity = NULL;
                    //     collinding_object->angularVelocity = NULL;
                    // }
                    // Causes stack overflow!!
                    free(last_collision_this_points);
                    free(last_collision_other_points);
                    break;
                }
            //}

            free(last_collision_this_points);
            free(last_collision_other_points);





            // foundCollision = foundCollision || collindig;
            
        }
    }

    free(transformedPoints);
    return data;
}

void Polygon::ApplyForce(Vector2D pos, Vector2D force, float deltaTime)
{
    ApplyAccel(pos, force / (Vector2D){mass, mass}, deltaTime);
}

Vector2D *Polygon::RotateAndTranslate()
{
    //Rotate
    Vector2D* points_rotated = (Vector2D*)malloc(num_points * sizeof(Vector2D));
    //Make relative to CM
    for(int i = 0; i < num_points; i++)
    {
        Vector2D rel_pos = points[i] - CenterOfMass;

        double absolute_value = sqrt((rel_pos.x * rel_pos.x) + (rel_pos.y * rel_pos.y));
        double point_angle = atan2(rel_pos.y , rel_pos.x);

        points_rotated[i] = {
            (float)(cos(point_angle + rotation) * absolute_value), //For some reason it is subtraction, I dont know why??
            (float)(sin(point_angle + rotation) * absolute_value)
        };
    }
    
    //Translate to position
    Vector2D* points_translated = (Vector2D*)malloc(num_points * sizeof(Vector2D));
    for(int i = 0; i < num_points; i++)
    {
        points_translated[i] = points_rotated[i] + position + CenterOfMass;
    }

    free(points_rotated);
    return points_translated;
}

void Polygon::ApplyAccel(Vector2D pos, Vector2D force, float deltaTime)
{
    Vector2D rel_pos = pos - CenterOfMass;

    rel_pos = rel_pos.rotate(rotation);
    force = force.rotate(rotation);

    //position += (velocity * deltaTime) + (force * (1.f/2) * deltaTime * deltaTime);
    velocity += force * deltaTime;

    float angularAccel = (dot(force, rel_pos.perpendicular()) / dot(rel_pos.perpendicular(), rel_pos.perpendicular()));
    //rotation += (angularVelocity * deltaTime) + (angularAccel * (1.f/2) * deltaTime * deltaTime);
    angularVelocity += angularAccel * deltaTime;


}

Vector2D Polygon::GetCenterOfMass()
{
    float area = GetArea();

    float x = 0;
    for(int i = 0; i < num_points; i++)
    {
        int next = (i + 1) % num_points;
        x += (points[i].x + points[next].x) * ((points[i].x * points[next].y) - (points[next].x * points[i].y));
    }
    x *= (1.f/ (6 * area));


    float y = 0;
    for(int i = 0; i < num_points; i++)
    {
        int next = (i + 1) % num_points;
        y += (points[i].y + points[next].y) * ((points[i].x * points[next].y) - (points[next].x * points[i].y));
    }
    y *= (1.f/ (6 * area));

    return {x, y};
}

float Polygon::GetArea()
{
    float area = 0;
    for(int i = 0; i < num_points - 1; i++)
    {
        int next = (i + 1) % num_points;
        area += (points[i].x * points[next].y) - (points[next].x * points[i].y);
    }
    area*=(1.f/2.f);

    return fabs(area);
}

float Polygon::GetMomentOfInertia()
{
    Vector2D* pointsCentered = (Vector2D*)malloc(num_points * sizeof(Vector2D));
    for(int i = 0; i < num_points; i++)
        pointsCentered[i] = points[i] - CenterOfMass;

    float J_x = 0;
    for(int i = 0; i < num_points; i++)
    {
        int next = (i + 1) % num_points;
        J_x += (pow(pointsCentered[i].y, 2) + (pointsCentered[i].y * pointsCentered[next].y) + pow(pointsCentered[next].y, 2)) * ((pointsCentered[i].x * pointsCentered[next].y) - (pointsCentered[next].x * pointsCentered[i].y));
    }
    J_x *= (1.f / 12);

    float J_y = 0;
    for(int i = 0; i < num_points; i++)
    {
        int next = (i + 1) % num_points;
        J_y += (pow(pointsCentered[i].x, 2) + (pointsCentered[i].x * pointsCentered[next].x) + pow(pointsCentered[next].x, 2)) * ((pointsCentered[i].x * pointsCentered[next].y) - (pointsCentered[next].x * pointsCentered[i].y));
    }
    J_y *= (1.f / 12);

    float J_z = J_x + J_y;
    float area = GetArea();
    float density = mass / area;
    float I_z = J_z * density;

    free(pointsCentered);
    return I_z;
}