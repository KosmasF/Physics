#pragma once
#include "Polygon.h"
#include <SDL2/SDL.h>

extern float deltaTime;

inline bool left_pressed = false;
inline bool right_pressed = false;
inline bool up_pressed = false;

bool Loop(Polygon* rocket_ptr);