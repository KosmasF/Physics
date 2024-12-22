#include "Units.h"
#include "../Chess/OpenCL/GPU.h"
#include "../Chess/NeuralNetwork/NeuralNetwork.h"
#include "../Chess/NeuralNetwork/ActivationMethods.h"
#include "SDL2/SDL.h"
#include "Object.h"
#include "Polygon.h"
#include "math.h"
#include <pthread.h>
#include <signal.h>
#include "Signals.h"
#include "hsl.h"

#define OBSTACLE_LAYER -1// Layer -1 is prohibited
#define RESTING_LAYER 0
#define ROCKET_LAYER 1// Layer 1 is rocket
#define DESTINATION_LAYER 2// Layer 2 is destination

#define OBSTACLE_LAYER_INPUT_ID 0
#define RESTING_LAYER_INPUT_ID 1
#define DESTINATION_LAYER_INPUT_ID 2
#define ROCKET_INPUT_ID 3
#define ROCKET_PREVIOUS_INPUT_ID 4

#define NUM_INPUT_LAYERS 5

#define INPUT_WIDTH 50
#define INPUT_HEIGHT 50

#define OUT_MOVE_RIGHT_ID 0
#define OUT_MOVE_LEFT_ID 1

#define DURATION_SEC 25
#define DELTA_TIME (1.f / 10) //60fps

#define BATCH_SIZE 30
#define SURVIVOR_NUM 10
#define RANDOM_NUM 10
#define EXPANSION_NUM 0

#define MAX_MUTATION_RATE 2
#define MAX_LAYER_NUM 4
#define MAX_LAYER_SIZE 128

#define REPLAY_SPEED 1

// #define PRINT_TIME

inline NeuralNetwork* survivors;

inline GPU gpu;

inline int layerSize[] = {INPUT_WIDTH * INPUT_HEIGHT * NUM_INPUT_LAYERS, 2};
inline float (*ActivationMethods[])(float) = {Sigmoid};
inline const bool normalizeOutput = true;
inline pthread_t play_thread_id;
inline bool play_thread_wait = false;
inline pthread_cond_t has_stopped;
inline pthread_mutex_t p_mutex;
inline bool play_thread_run = true;

extern Polygon ground;                     // = Polygon({0, HEIGHT - 10}, M_PIf, INFINITY, 0, 0, 0, ground_collides, sizeof(ground_collides) / sizeof(int) ,ground_points, sizeof(ground_points) / sizeof(Vector2D), RED);
extern Polygon right_wall;                 // = Polygon({-WIDTH / 2 + 5, HEIGHT / 2}, M_PIf / 2, INFINITY, 0, 1, -1, ground_collides, sizeof(ground_collides) / sizeof(int) , ground_points, sizeof(ground_points) / sizeof(Vector2D), RED);
extern Polygon left_wall;                  // = Polygon({+WIDTH / 2 - 5, HEIGHT / 2}, M_PIf / 2, INFINITY, 0, 1, -1, ground_collides, sizeof(ground_collides) / sizeof(int) , ground_points, sizeof(ground_points) / sizeof(Vector2D), RED);
extern Polygon fallingPolygon;             // = Polygon({WIDTH/ 2, 0},  M_PIf / 6, 1, 1, 0.5f, 0, objects_collides, sizeof(objects_collides) / sizeof(int), square, sizeof(square) / sizeof(Vector2D), {0xff, 0xff, 0x00, 0xff});
extern Polygon fallingPolygon2;            // = Polygon({WIDTH/ 2 - 25, -300},  0, 1, 1, 0.5f, 0, objects_collides, sizeof(objects_collides) / sizeof(int), square, sizeof(square) / sizeof(Vector2D), {0x00, 0xff, 0x00, 0xff});
extern Polygon rocket;                     // = Polygon({WIDTH/2 - 12.5f, HEIGHT - 50 - 10}, 0, 1, 1, 0, 1, rocket_collides, sizeof(rocket_collides)/ sizeof(int), parallerogram, sizeof(parallerogram) / sizeof(Vector2D), BLUE);
extern Polygon target;                     // = Polygon({WIDTH/2 - 50, 10}, 0, INFINITY, 0, 0, 2, ground_collides, sizeof(ground_collides) / sizeof(int), square, sizeof(square) / sizeof(Vector2D), GREEN);
extern Polygon wall;                       // = Polygon({WIDTH / 4, HEIGHT/4}, 0, INFINITY, 0, 0, -1, ground_collides, sizeof(ground_collides) / sizeof(int), obstacle, sizeof(obstacle) / sizeof(Vector2D), RED);

extern SDL_Window* win;
extern SDL_mutex* mutex;

extern char* filename;
extern char* survivor_filename;

inline Object** current_scene_objects = nullptr;
inline int num_objects = 0;

inline float* previous_frame = nullptr;


struct PlayerData
{
    bool lifted_off = false;
    float max_eval = 0;
    bool collided = false;
    bool succeded = false;
};

void setup(Vector2D offset);

void generation();
float simulate(NeuralNetwork* nn);

void terminate();

float GetEval(Vector2D pos, Vector2D target_pos, bool collided, bool succedeed);

void play(NeuralNetwork* nn, SDL_Window* win, Vector2D offset);
void DrawScene(Object**& scene, int& num_objects, SDL_Window* win, SDL_Renderer* renderer, Vector2D offset);

void* PlayBest(void* args);
void* PlayCurrent(void* args);