#include "Loop.h"

bool Loop(Polygon* rocket_ptr) {
	SDL_Event e;

	while( SDL_PollEvent( &e ) != 0 ) {
        if(e.type == SDL_QUIT)
        {
            return false;
        }

        if(e.type == SDL_KEYDOWN)
        {
            if(e.key.keysym.sym == SDLK_RIGHT)
            {
               right_pressed = true;
            }
            if(e.key.keysym.sym == SDLK_LEFT)
            {
                left_pressed = true;
            }
            if(e.key.keysym.sym == SDLK_UP)
            {
               up_pressed = true;
            }
        }
        if(e.type == SDL_KEYUP)
        {
            if(e.key.keysym.sym == SDLK_RIGHT)
            {
               right_pressed = false;
            }
            if(e.key.keysym.sym == SDLK_LEFT)
            {
                left_pressed = false;
            }
            if(e.key.keysym.sym == SDLK_UP)
            {
                up_pressed = false;
            }
        }
	}

    if(right_pressed || up_pressed)
    {
        if(rocket_ptr)
        rocket_ptr->ApplyAccel({25, 50}, {GRAVITY, -GRAVITY}, deltaTime);
    }
    if(left_pressed || up_pressed)
    {
        if(rocket_ptr)
        rocket_ptr->ApplyAccel({0, 50}, {-GRAVITY, -GRAVITY}, deltaTime);
    }

    return true;
}