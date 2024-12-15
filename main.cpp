#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "time.h"
#include <string.h>

#include "Loop.h"
#include "Object.h"
#include "Circle.h"
#include "Polygon.h"
#include "Signals.h"

#include "player.h"

#define MAX_ARGS 5
#define GLOBAL_OFFSET (Vector2D){0, 250}

// maxint
SDL_Window* win;
SDL_mutex* mutex;

// Polygon* rocket_ptr;
// Polygon* target_ptr;
float deltaTime;
// Object** objects_ptr;
// int num_objects;

char* filename = nullptr;
char* survivor_filename = nullptr;
bool commands_from_file = false;

//Circle circle = Circle({WIDTH / 2, 100}, 0, 1, 100, 10, {0xff, 0x00, 0x00, 0xff});
Vector2D square[] = {//formerly points[]
    {0,0},
    {100,0},
    {100,100},
    {0,100}
};
Vector2D ground_points[] = {
    {0,0},
    {WIDTH,0},
    {WIDTH,10},
    {0,10}
};
Vector2D parallerogram[] = {
    {0,0},
    {25,0},
    {25,50},
    {0,50}
};
Vector2D obstacle[] = {
    {0,0},
    {WIDTH / 2.f, 0},
    {WIDTH / 2.f, 10},
    {0,10}
};

int ground_collides[] = {};
int objects_collides[] = {-1, 0, 1, 2};
int rocket_collides[] =  {-1, 0, 1, 2};
// Polygon polygon = Polygon({WIDTH/ 2 - 50, HEIGHT - 11},  M_PIf / 4, INFINITY, 0,1, ground_points, sizeof(points) / sizeof(Vector2D), {0x00, 0x00, 0xff, 0xff});
Polygon ground = Polygon({0, HEIGHT - 10}, M_PIf, INFINITY, 0, 0.1f, 0, ground_collides, sizeof(ground_collides) / sizeof(int) ,ground_points, sizeof(ground_points) / sizeof(Vector2D), {0xff,0xff,0xff,0xff});
Polygon right_wall = Polygon({-WIDTH / 2 + 5, HEIGHT / 2}, M_PIf / 2, INFINITY, 0, 1, -1, ground_collides, sizeof(ground_collides) / sizeof(int) , ground_points, sizeof(ground_points) / sizeof(Vector2D), RED);
Polygon left_wall = Polygon({+WIDTH / 2 - 5, HEIGHT / 2}, M_PIf / 2, INFINITY, 0, 1, -1, ground_collides, sizeof(ground_collides) / sizeof(int) , ground_points, sizeof(ground_points) / sizeof(Vector2D), RED);
Polygon fallingPolygon = Polygon({WIDTH/ 2, 0},  M_PIf / 6, 1, 1, 0.5f, 0, objects_collides, sizeof(objects_collides) / sizeof(int), square, sizeof(square) / sizeof(Vector2D), {0xff, 0xff, 0x00, 0xff});
Polygon fallingPolygon2 = Polygon({WIDTH/ 2 - 25, -300},  0, 1, 1, 0.5f, 0, objects_collides, sizeof(objects_collides) / sizeof(int), square, sizeof(square) / sizeof(Vector2D), {0x00, 0xff, 0x00, 0xff});
Polygon rocket = Polygon({WIDTH/2 - 12.5f, HEIGHT - 50 - 10}, 0, 1, 1, 0, 1, rocket_collides, sizeof(rocket_collides)/ sizeof(int), parallerogram, sizeof(parallerogram) / sizeof(Vector2D), BLUE);
Polygon target = Polygon({WIDTH/2 - 50, 10}, 0, INFINITY, 0, 0, 2, ground_collides, sizeof(ground_collides) / sizeof(int), square, sizeof(square) / sizeof(Vector2D), GREEN);
Polygon wall = Polygon({WIDTH / 4, HEIGHT/4}, 0, INFINITY, 0, 0, -1, ground_collides, sizeof(ground_collides) / sizeof(int), obstacle, sizeof(obstacle) / sizeof(Vector2D), RED);
//fallingPolygon.velocity = {100, 0};

int main(int argc, char** argv)
{
    Signals_Setup();


    int argc_copy = argc;
    char** argv_copy = (char**)malloc(argc * sizeof(char *));
    memcpy(argv_copy, argv, argc * sizeof(char *));

    int ret;
	ret = SDL_Init( SDL_INIT_EVERYTHING );
    if (ret != 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return -1;
    }
    win = SDL_CreateWindow("Main", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH + GLOBAL_OFFSET.x, HEIGHT + GLOBAL_OFFSET.y, SDL_WINDOW_OPENGL);
    // renderer = SDL_CreateRenderer(win, 0, 0);
    mutex = SDL_CreateMutex();

    Polygon a = Polygon(ground);Polygon b =  Polygon(right_wall);Polygon c = Polygon(left_wall);Polygon d = Polygon(rocket);Polygon e = Polygon(target);Polygon f = Polygon(wall);
    Object* objects[] = {&a, &b, &c, &d, &e, &f};

    READ_COMMAND:
    if(argc_copy > 1)
    {
        if(!strcmp(argv_copy[1], "play"))
        {
            SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, 0);
            float start_time = clock() / (float)CLOCKS_PER_SEC;
            deltaTime = 0;
            while(Loop((Polygon*)objects[3]))
            {
                for(int i = 0; i < sizeof(objects) / sizeof(Object*); i++)
                {
                    objects[i]->Update(deltaTime);
                    objects[i]->CheckForCollision(objects, sizeof(objects) / sizeof(Object*), deltaTime);
                }

                // fallingPolygon.ApplyAccel({50, 0}, {0 , -GRAVITY}, deltaTime);
                //rocket.ApplyAccel({12.5f, 0}, {0, -(GRAVITY)}, deltaTime);

                SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
                SDL_RenderClear(renderer);

                for(int i = 0; i < sizeof(objects) / sizeof(Object*); i++)
                {
                    objects[i]->Draw(win, GLOBAL_OFFSET);
                }

                SDL_RenderPresent(renderer);
                SDL_UpdateWindowSurface(win);

                float end_time = clock() / (float)CLOCKS_PER_SEC;
                deltaTime = end_time - start_time;
                start_time = end_time;
            }
            SDL_DestroyRenderer(renderer);
        }
        else if(!strcmp(argv_copy[1], "train"))
        {
            if(argc_copy > 2)
            {
                filename = argv_copy[2];
            }
            if(argc_copy > 3)
            {
                survivor_filename = argv_copy[3];
            }
            setup(GLOBAL_OFFSET);

            while(Loop(nullptr))
            {
                generation();
            }

            terminate();
        }
        else if(!strcmp(argv_copy[1], "show"))
        {
            if(argc_copy > 2)
            {
                NeuralNetwork nn(argv_copy[2], &gpu);
                play(&nn, win, GLOBAL_OFFSET);
                while(Loop(nullptr)){}
            }
            else
            {
                printf("Filename not found!\n");
            }
        }
        else
        {
            printf("Invalid command: %s\n", argv_copy[1]);
        }
        FILE* file = fopen("last_command.txt", "w");
        for(int i = 0; i < argc_copy; i++)
        {
            fwrite(argv_copy[i], strlen(argv_copy[i]), 1, file);
            fwrite(" ", strlen(" "), 1, file);
        }
        fclose(file);
    }
    else
    {
        printf("Command not found!\n");
        printf("Loading last command!\n");
        commands_from_file = true;

        FILE* file = fopen("last_command.txt", "r");
        fseek(file, 0, SEEK_END);
        int size = ftell(file);
        fseek(file, 0, SEEK_SET);
        char* content = (char*)malloc(size + 1);
        fread(content, size, 1, file);
        content[size] = 0;
        fclose(file);

        char *args[MAX_ARGS];
        int arg_count;

        char *input_copy = strdup(content);
        char *token;
        arg_count = 0;

        token = strtok(input_copy, " ");
        while (token != NULL && arg_count < MAX_ARGS) {
            args[arg_count] = strdup(token);
            arg_count++;
            token = strtok(NULL, " ");
        }

        free(input_copy);
        free(argv_copy);

        argc_copy = arg_count;
        argv_copy = args;
        free(content);
        goto READ_COMMAND;
    }

    if(!commands_from_file)
    {
        free(argv_copy);
    }
    else
    {
        for(int i = 0; i < argc_copy; i++)
            free(argv_copy[i]);
    }

    SDL_DestroyMutex(mutex);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}