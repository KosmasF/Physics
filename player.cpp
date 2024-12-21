#include "player.h"

void setup(Vector2D offset)
{
    srand(time(NULL));
    survivors = (NeuralNetwork*)malloc(SURVIVOR_NUM * sizeof(NeuralNetwork));

    int i = 0;
    if(survivor_filename)
    {
        new (&survivors[0]) NeuralNetwork(survivor_filename, &gpu);
        i++;
    }

    for(; i < SURVIVOR_NUM; i++)
    {
        new (&survivors[i]) NeuralNetwork(layerSize, sizeof(layerSize) / sizeof(int), ActivationMethods, normalizeOutput, &gpu);
    }

    previous_frame = (float*)malloc(INPUT_HEIGHT * INPUT_HEIGHT * sizeof(float));

   
    printf("Play thread started.\n");
    pthread_create(&play_thread_id, nullptr, PlayCurrent, (void*)&offset);
    // pthread_join(play_thread_id, NULL);
}

void terminate()
{
    if(filename)
        survivors[0].Save(filename);
    for(int i = 0; i < SURVIVOR_NUM; i++)
        survivors[i].~NeuralNetwork();
    free(survivors);
    free(previous_frame);


    play_thread_run = false;
    pthread_join(play_thread_id, nullptr);
    printf("Play thread terminated.\n");
    // Maybe save the best machines, but do that later!
}

float GetEval(Vector2D pos, Vector2D target_pos, bool collided, bool succeded)
{
    //CALCULATE LOSS OF THE NN
    float eval = 0;
    if(!collided)
        eval += 50;
    if(succeded)
        eval += 100000;

    Vector2D relative_pos = pos - target_pos;
    float angle = relative_pos.perpendicular().perpendicular().perpendicular().angle();
    eval += fabs(angle) * RADIAN;

    float distance = (relative_pos / (Vector2D){WIDTH, HEIGHT}).len() / 10.f;
    eval -= distance;

    return eval;
}

void generation()
{
    static bool first_it = true;
    NeuralNetwork* neuralNetworks = (NeuralNetwork*)malloc(BATCH_SIZE * sizeof(NeuralNetwork));
    for(int i = 0; i < SURVIVOR_NUM; i++)
    {
        new (&neuralNetworks[i]) NeuralNetwork(survivors[i]);
    }

    for(int i = SURVIVOR_NUM; i < BATCH_SIZE; i++)
    {
        float muation_rate = (((float)(rand()) / RAND_MAX)) * MAX_MUTATION_RATE;
        if(i < (BATCH_SIZE - RANDOM_NUM - EXPANSION_NUM))
            new (&neuralNetworks[i]) NeuralNetwork(survivors[i % SURVIVOR_NUM]);
        else if(i < (BATCH_SIZE - EXPANSION_NUM))
        {
            int _layer_num = (rand() % (MAX_LAYER_NUM - 1)) + 1 + 2;
            int* _layer_size = (int*)malloc(_layer_num * sizeof(int));
            for(int l = 1; l < _layer_num - 1; l++)
            {
                _layer_size[l] = (rand() % (MAX_LAYER_SIZE - 1)) + 1;
            }
            _layer_size[0] = INPUT_HEIGHT * INPUT_WIDTH * NUM_INPUT_LAYERS;
            _layer_size[_layer_num - 1] = 2;
            float (*ActivationMethods[_layer_num - 1])(float);
            for(int a = 0; a < _layer_num - 1; a++)
            {
                ActivationMethods[a] = (rand() % 2) ? None : Sigmoid;
            }
            ActivationMethods[_layer_num - 2] = Sigmoid;
            new (&neuralNetworks[i]) NeuralNetwork(_layer_size, _layer_num, ActivationMethods, normalizeOutput, &gpu);
            free(_layer_size);
        }
        else
        {
            NeuralNetwork* nn = &survivors[i % SURVIVOR_NUM];
            int layer_added = (rand() % (MAX_LAYER_NUM - nn->LayerNum + 1));
            int _layer_num = nn->LayerNum + layer_added;
            int* _layer_size = (int*)malloc(_layer_num * sizeof(int));
            for(int l = 1; l < _layer_num - 1; l++)
            {
                if(l < nn->LayerNum)
                    _layer_size[l] = nn->LayerSize[l] + (rand() % (MAX_LAYER_NUM - nn->LayerSize[l] + 1));
                else
                    _layer_size[l] = (rand() % (MAX_LAYER_NUM - 1)) + 1;
            }
            _layer_size[0] = INPUT_HEIGHT * INPUT_WIDTH * NUM_INPUT_LAYERS;
            _layer_size[_layer_num - 1] = 2;
            float (*ActivationMethods[_layer_num - 1])(float);
            for(int a = 0; a < _layer_num - 1; a++)
            {
                if(a < nn->LayerNum - 1)
                    ActivationMethods[a] = nn->ActivationMethods[a];
                else
                    ActivationMethods[a] = (rand() % 2) ? None : Sigmoid;
            }
            ActivationMethods[_layer_num - 2] = Sigmoid;
            new (&neuralNetworks[i]) NeuralNetwork(_layer_size, _layer_num, ActivationMethods, normalizeOutput, &gpu);
            NeuralNetwork* nn_new = &neuralNetworks[i];
            // nn_new->PrintArchitecture();

            // int weight_buffer = 0;
            // int idx = 0;
            for (int layer = 1; layer < _layer_num; layer++)
            {
                for (int nl = 0; nl < _layer_size[layer]; nl++)
                {
                    if(layer < nn->LayerNum)
                        if(nl < nn->LayerSize[layer])
                            nn_new->weights[nn_new->weights_buffer_lookup_table[nn_new->GetIndexOfNeuron(layer, nl)]] = nn->weights[nn->weights_buffer_lookup_table[nn->GetIndexOfNeuron(layer, nl)]];//I saved a bug
                        else
                            nn_new->weights[nn_new->weights_buffer_lookup_table[nn_new->GetIndexOfNeuron(layer, nl)]] = (((float)(rand()) / RAND_MAX) - (1 / 2.f)) * muation_rate;
                    else
                        nn_new->weights[nn_new->weights_buffer_lookup_table[nn_new->GetIndexOfNeuron(layer, nl)]] = (((float)(rand()) / RAND_MAX) - (1 / 2.f)) * muation_rate;

                    // weights_buffer_lookup_table[idx] = weight_buffer;
                    // weight_buffer += layerSize[layer - 1];
                }
            }
            free(_layer_size);
            continue;
        }
        float weights_num = neuralNetworks[i].GetNumberOfWeights() ;
        float* mutation = (float*)malloc(weights_num * sizeof(float));
        for(int n = 0; n < weights_num; n++)
            mutation[n] = (((float)(rand()) / RAND_MAX) - (1 / 2.f)) * muation_rate;
        neuralNetworks[i].AddToWeights(mutation);
        free(mutation);
    }

    float* evals = (float*)malloc(BATCH_SIZE * sizeof(float));

    for(int batch = first_it ? 0 : SURVIVOR_NUM; batch < BATCH_SIZE; batch++)
    {
        evals[batch] = simulate(&neuralNetworks[batch]);
    }

    static float max_eval[SURVIVOR_NUM];
    static int max_eval_id[SURVIVOR_NUM];

    for (int i = 0; first_it ? i < SURVIVOR_NUM : false; i++)
        max_eval[i] = -INFINITY;
   
    for (int eval = first_it ? 0 : SURVIVOR_NUM; eval < BATCH_SIZE; eval++) {
        for (int i = 0; i < SURVIVOR_NUM; i++) {
            if (evals[eval] >= max_eval[i]) {
                
                for (int j = SURVIVOR_NUM - 1; j > i; j--) {
                    max_eval[j] = max_eval[j - 1];
                    max_eval_id[j] = max_eval_id[j - 1];
                }
               
                max_eval[i] = evals[eval];
                max_eval_id[i] = eval;
                break;
            }
        }
    }

    for(int i = 0; i < SURVIVOR_NUM; i++)
    {
        survivors[i].~NeuralNetwork();
        new (&survivors[i]) NeuralNetwork(neuralNetworks[max_eval_id[i]]);
    }
    for(int i = 0; i < SURVIVOR_NUM; i++)
        max_eval_id[i] = i;


    free(evals);
    // SDL_Surface* windowSurface = SDL_GetWindowSurface(win);

    for(int i = 0; i < BATCH_SIZE; i++)
        neuralNetworks[i].~NeuralNetwork();
    free(neuralNetworks);
    
    printf("Survivors: ");
    for(int i = 0; i < SURVIVOR_NUM; i++)
        printf("%f ", max_eval[i]);
    printf("\n");
    first_it = false;
}

float simulate(NeuralNetwork *nn)
{
    Polygon a(ground), b(right_wall), c(left_wall), d(rocket), e(target), f(wall);
    Object* objects[] = {&a, &b, &c, &d, &e, &f};
    current_scene_objects = objects;
    num_objects = sizeof(objects) / sizeof(Object*);
    
    PlayerData player_data;

    float time;
    for(time = 0; time < DURATION_SEC; time+=DELTA_TIME)
    {
        // DRAW SCREEN
        TimeSetZero();
        float* input = (float*)malloc(NUM_INPUT_LAYERS * INPUT_HEIGHT * INPUT_WIDTH * sizeof(float));
        for(int i = 0; i < NUM_INPUT_LAYERS * INPUT_HEIGHT * INPUT_WIDTH; i++)
            input[i] = 0;
        //input[NUM_INPUT_LAYERS * INPUT_HEIGHT * INPUT_WIDTH] = FLT_MAX;

        // OBSTACLE LAYER   
        for(int i = 0; i < sizeof(objects) / sizeof(Object*); i++)
        {
            if(((Polygon*)objects[i])->ReturnCollisionLayer() == OBSTACLE_LAYER)
            {
                ((Polygon*)objects[i])->DrawInArray(input + (OBSTACLE_LAYER_INPUT_ID * INPUT_HEIGHT * INPUT_WIDTH), HEIGHT, WIDTH, INPUT_HEIGHT, INPUT_WIDTH);
            }
            else if(((Polygon*)objects[i])->ReturnCollisionLayer() == RESTING_LAYER)
            {
                ((Polygon*)objects[i])->DrawInArray(input + (RESTING_LAYER_INPUT_ID * INPUT_HEIGHT * INPUT_WIDTH), HEIGHT, WIDTH, INPUT_HEIGHT, INPUT_WIDTH);
            }
            else if(((Polygon*)objects[i])->ReturnCollisionLayer() == DESTINATION_LAYER)
            {
                ((Polygon*)objects[i])->DrawInArray(input + (DESTINATION_LAYER_INPUT_ID * INPUT_HEIGHT * INPUT_WIDTH), HEIGHT, WIDTH, INPUT_HEIGHT, INPUT_WIDTH);
            }
            else if(((Polygon*)objects[i])->ReturnCollisionLayer() == ROCKET_LAYER)
            {
                ((Polygon*)objects[i])->DrawInArray(input + (ROCKET_INPUT_ID * INPUT_HEIGHT * INPUT_WIDTH), HEIGHT, WIDTH, INPUT_HEIGHT, INPUT_WIDTH);
            }
        }
        if(time != 0)
            memcpy(input + (ROCKET_PREVIOUS_INPUT_ID * INPUT_HEIGHT * INPUT_WIDTH), previous_frame, INPUT_HEIGHT * INPUT_HEIGHT * sizeof(float));
        memcpy(previous_frame, input + (ROCKET_INPUT_ID * INPUT_HEIGHT * INPUT_WIDTH), INPUT_HEIGHT * INPUT_HEIGHT * sizeof(float));

        // TimeElapsed("Input setup: ");
        float* output = nn->Generate(input, false);
        free(input);
        // TimeElapsed("Generate input: ");
        //float* output = nullptr;
        
        // printf("%f %f\n", output[OUT_MOVE_RIGHT_ID], output[OUT_MOVE_LEFT_ID]);
        // rocket_ptr->ApplyAccel({25, 50}, {GRAVITY * min(max((output[OUT_MOVE_RIGHT_ID]), 0), 1), -GRAVITY * min(max((output[OUT_MOVE_RIGHT_ID]), 0), 1)}, DELTA_TIME);
        // rocket_ptr->ApplyAccel({0, 50}, {-GRAVITY * min(max((output[OUT_MOVE_LEFT_ID]), 0), 1), -GRAVITY * min(max((output[OUT_MOVE_LEFT_ID]), 0), 1)}, DELTA_TIME);
        ((Polygon*)objects[3])->ApplyAccel({25, 50}, {GRAVITY * output[OUT_MOVE_RIGHT_ID], -GRAVITY * output[OUT_MOVE_RIGHT_ID]}, DELTA_TIME);
        ((Polygon*)objects[3])->ApplyAccel({0, 50}, {-GRAVITY * output[OUT_MOVE_LEFT_ID], -GRAVITY * output[OUT_MOVE_LEFT_ID]}, DELTA_TIME);
        free(output);

        //if(output)

        // SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
        // SDL_RenderClear(renderer);

        // for(int i = 0; i < num_objects; i++)
        // {
        //     objects_ptr[i]->Draw(win);
        // }

        // SDL_RenderPresent(renderer);
        // SDL_UpdateWindowSurface(win);

        // //DECIDE CHOICE
        // SDL_Surface* windowSurface = SDL_GetWindowSurface(win);
        // windowSurface->pixels;//pixles :)


        //UPDATE
        for(int i = 0; i < sizeof(objects) / sizeof(Object*); i++)
        {
            objects[i]->Update(DELTA_TIME);
            CollisionsData data =((Polygon*)objects[i])->CheckForCollision(objects, sizeof(objects) / sizeof(Object*), DELTA_TIME);
            bool in_contact_with_ground = false;
            if(data.num > 0)
            {
                for(int i = 0; i < data.num; i++)
                {
                    if(data.data[i].collision_layer_1 == ROCKET_LAYER || data.data[i].collision_layer_2 == ROCKET_LAYER)
                    {
                        if(data.data[i].collision_layer_1 == OBSTACLE_LAYER || data.data[i].collision_layer_2 == OBSTACLE_LAYER)
                        {
                            player_data.collided = true;
                            goto EVAL;
                        }   
                        else if(data.data[i].collision_layer_1 == DESTINATION_LAYER || data.data[i].collision_layer_2 == DESTINATION_LAYER)
                        {
                            player_data.succeded = true;
                            goto EVAL;
                        }
                        else if(data.data[i].collision_layer_1 == RESTING_LAYER || data.data[i].collision_layer_2 == RESTING_LAYER)
                        {
                            if(!player_data.lifted_off)
                                in_contact_with_ground = true;
                            else{
                                player_data.collided = true;
                                goto EVAL;
                            }
                        }
                    }
                }
            }
            if(!in_contact_with_ground)
                player_data.lifted_off = true;
        }
        float eval = GetEval(((Polygon*)objects[3])->ReturnPosition(), ((Polygon*)objects[4])->ReturnPosition(), true, false);
        if(eval > player_data.max_eval)
            player_data.max_eval = eval;
        // TimeElapsed("Physics: ");
    }

EVAL:
    float eval = GetEval(((Polygon*)objects[3])->ReturnPosition(), ((Polygon*)objects[4])->ReturnPosition(), player_data.collided, player_data.succeded);

    eval += player_data.max_eval;
    // eval -= abs(((Polygon*)objects[3])->ReturnRotation()) * RADIAN;
    if(player_data.succeded)
    eval -= time;
    else
        eval += time;

    // printf("Eval: %f\n", eval);
    num_objects = 0;
    pthread_mutex_lock(&p_mutex);
    pthread_kill(play_thread_id, SIGHOLD);
    pthread_cond_wait(&has_stopped, &p_mutex);
    pthread_mutex_unlock(&p_mutex);
    // printf("Eval ended!\n");
    return eval;
}

void play(NeuralNetwork* nn, SDL_Window* win, Vector2D offset)
{
    previous_frame = (float*)malloc(INPUT_HEIGHT * INPUT_HEIGHT * sizeof(float));
    SDL_LockMutex(mutex);
    SDL_GLContext context = SDL_GL_CreateContext(win);
    SDL_GL_MakeCurrent(win, context);
    SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, 0);


    Polygon a(ground), b(right_wall), c(left_wall), d(rocket), e(target), f(wall);
    Object* objects[] = {&a, &b, &c, &d, &e, &f};

    for(float time = 0; time < DURATION_SEC; time+=DELTA_TIME)
    {
        float start_time = clock() / (float)CLOCKS_PER_SEC;

        // DRAW SCREEN
        float* input = (float*)malloc(NUM_INPUT_LAYERS * INPUT_HEIGHT * INPUT_WIDTH * sizeof(float));
        for(int i = 0; i < NUM_INPUT_LAYERS * INPUT_HEIGHT * INPUT_WIDTH; i++)
            input[i] = 0;
        //input[NUM_INPUT_LAYERS * INPUT_HEIGHT * INPUT_WIDTH] = FLT_MAX;

        // OBSTACLE LAYER   
        for(int i = 0; i < sizeof(objects) / sizeof(Object*); i++)
        {
            if(((Polygon*)objects[i])->ReturnCollisionLayer() == OBSTACLE_LAYER)
            {
                ((Polygon*)objects[i])->DrawInArray(input + (OBSTACLE_LAYER_INPUT_ID * INPUT_HEIGHT * INPUT_WIDTH), HEIGHT, WIDTH, INPUT_HEIGHT, INPUT_WIDTH);
            }
            else if(((Polygon*)objects[i])->ReturnCollisionLayer() == RESTING_LAYER)
            {
                ((Polygon*)objects[i])->DrawInArray(input + (RESTING_LAYER_INPUT_ID * INPUT_HEIGHT * INPUT_WIDTH), HEIGHT, WIDTH, INPUT_HEIGHT, INPUT_WIDTH);
            }
            else if(((Polygon*)objects[i])->ReturnCollisionLayer() == DESTINATION_LAYER)
            {
                ((Polygon*)objects[i])->DrawInArray(input + (DESTINATION_LAYER_INPUT_ID * INPUT_HEIGHT * INPUT_WIDTH), HEIGHT, WIDTH, INPUT_HEIGHT, INPUT_WIDTH);
            }
            else if(((Polygon*)objects[i])->ReturnCollisionLayer() == ROCKET_LAYER)
            {
                ((Polygon*)objects[i])->DrawInArray(input + (ROCKET_INPUT_ID * INPUT_HEIGHT * INPUT_WIDTH), HEIGHT, WIDTH, INPUT_HEIGHT, INPUT_WIDTH);
            }
        }
        if(time != 0)
            memcpy(input + (ROCKET_PREVIOUS_INPUT_ID * INPUT_HEIGHT * INPUT_WIDTH), previous_frame, INPUT_HEIGHT * INPUT_HEIGHT * sizeof(float));
        memcpy(previous_frame, input + (ROCKET_INPUT_ID * INPUT_HEIGHT * INPUT_WIDTH), INPUT_HEIGHT * INPUT_HEIGHT * sizeof(float));

        
        float* output = nn->Generate(input, false);
        free(input);
        //float* output = nullptr;
        

        // rocket_ptr->ApplyAccel({25, 50}, {GRAVITY * min(max((output[OUT_MOVE_RIGHT_ID]), 0), 1), -GRAVITY * min(max((output[OUT_MOVE_RIGHT_ID]), 0), 1)}, DELTA_TIME);
        // rocket_ptr->ApplyAccel({0, 50}, {-GRAVITY * min(max((output[OUT_MOVE_LEFT_ID]), 0), 1), -GRAVITY * min(max((output[OUT_MOVE_LEFT_ID]), 0), 1)}, DELTA_TIME);
        ((Polygon*)objects[3])->ApplyAccel({25, 50}, {GRAVITY * max(0, output[OUT_MOVE_RIGHT_ID]), -GRAVITY* max(0, output[OUT_MOVE_RIGHT_ID])}, DELTA_TIME);
        ((Polygon*)objects[3])->ApplyAccel({0, 50}, {-GRAVITY * max(0, output[OUT_MOVE_LEFT_ID]), -GRAVITY* max(0, output[OUT_MOVE_LEFT_ID])}, DELTA_TIME);
        free(output);

        //if(output)

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
        SDL_RenderClear(renderer);

        for(int i = 0; i < sizeof(objects) / sizeof(Object*); i++)
        {
            objects[i]->Draw(win, offset);
        }

        SDL_RenderPresent(renderer);
        SDL_UpdateWindowSurface(win);

        //UPDATE
        for(int i = 0; i < sizeof(objects) / sizeof(Object*); i++)
        {
            objects[i]->Update(DELTA_TIME);
            CollisionsData data = ((Polygon*)objects[i])->CheckForCollision(objects,  sizeof(objects) / sizeof(Object*), DELTA_TIME);
            if(data.num > 0)
            {
                for(int i = 0; i < data.num; i++)
                {
                    if(data.data[i].collision_layer_1 == ROCKET_LAYER || data.data[i].collision_layer_2 == ROCKET_LAYER)
                    {
                        if(data.data[i].collision_layer_1 == OBSTACLE_LAYER || data.data[i].collision_layer_2 == OBSTACLE_LAYER)
                        {
                            return;
                        }   
                        else if(data.data[i].collision_layer_1 == DESTINATION_LAYER || data.data[i].collision_layer_2 == DESTINATION_LAYER)
                        {
                            return;
                        }
                    }
                }
            }
        }
        float end_time = clock() / (float)CLOCKS_PER_SEC;
        float res_time = ((DELTA_TIME / REPLAY_SPEED) - (end_time - start_time));
        // struct timespec req;
        // req.tv_sec = 0;
        // req.tv_nsec =  (DELTA_TIME - (end_time - start_time)) * 1000 * 1000000L; // 500 ms = 500,000,000 ns
        usleep(max(0, res_time * 1000 * 1000));
        // nanosleep(&req, NULL);
    }
    SDL_DestroyRenderer(renderer);
    SDL_GL_DeleteContext(context);
    SDL_UnlockMutex(mutex);
}

void DrawScene(Object**& scene, int& num_objects, SDL_Window *win, SDL_Renderer* renderer, Vector2D offset)
{
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
    SDL_RenderClear(renderer);

    for(int i = 0; i < num_objects; i++)
    {
        scene[i]->Draw(win, offset);
        if(play_thread_wait)
            goto SEND_SIGNAL;
    }

    SEND_SIGNAL:
    if(play_thread_wait)
    {
        pthread_cond_signal(&has_stopped);
        play_thread_wait = false;
    }

    SDL_RenderPresent(renderer);
    SDL_UpdateWindowSurface(win);
}

void* PlayCurrent(void * args)
{
    Vector2D offset = ((Vector2D*)args)[0];
    SDL_LockMutex(mutex);
    SDL_GLContext context = SDL_GL_CreateContext(win);
    SDL_GL_MakeCurrent(win, context);
    SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, 0);
    while(play_thread_run)
    {
        DrawScene(current_scene_objects, num_objects ,win, renderer, offset);
    }
    SDL_DestroyRenderer(renderer);
    SDL_GL_MakeCurrent(NULL, NULL);
    SDL_GL_DeleteContext(context);
    SDL_UnlockMutex(mutex); 
    return nullptr;
}

void* PlayBest(void* args)
{
    Vector2D offset = ((Vector2D*)args)[0];
    while(true)
    {
        NeuralNetwork nn = survivors[0];
        play(&nn, win,offset);
    }
    return nullptr;
}