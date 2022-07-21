//
//  GameProj3.cpp
//  SDLProject
//
//  Created by Nathan Atherley on 7/12/22.
//  Copyright © 2022 ctg. All rights reserved.
//

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define PLATFORM_COUNT 40
#define PAD_COUNT 5
//#define WALL_COUNT 2

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"

/**
 STRUCTS AND ENUMS
 */
struct GameState
{
    Entity* player;
    Entity* platforms;
    Entity* pad;
};

/**
 CONSTANTS
 */
const int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480,
          FONTBANK_SIZE = 16;
 //
const float BG_RED     = 0.1922f,
            BG_BLUE    = 0.549f,
            BG_GREEN   = 0.9059f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;
const char SPRITESHEET_FILEPATH[] = "more_ships.png"; // ship
const char PLATFORM_FILEPATH[]    = "rock_platform.png"; // bad platfform
const char PAD_FILEPATH[]         = "plat.png"; // good platform
const char FONT_FILEPATH[]        = "font1.png"; // font spread sheet


const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL  = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER   = 0;   // this value MUST be zero

/**
 VARIABLES
 */
GameState state;

SDL_Window* display_window;
bool game_is_running = true;
bool flag = false;

ShaderProgram program;
glm::mat4 view_matrix, projection_matrix;

float previous_ticks = 0.0f;
float accumulator = 0.0f;

float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
float delta_time = ticks - previous_ticks;
float curr_val = 0;
float curr_ticks = ticks;

/**
 GENERAL FUNCTIONS
 */
GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
         assert(false);
    }
    
    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    // STEP 3: Setting our texture filter modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // STEP 4: Setting our texture wrapping modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // the last argument can change depending on what you are looking for
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,  GL_REPEAT);
    
    // STEP 5: Releasing our file from memory and returning our texture id
    stbi_image_free(image);
    
    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    display_window = SDL_CreateWindow("Lunar Landing",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(display_window);
    SDL_GL_MakeCurrent(display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    program.Load(V_SHADER_PATH, F_SHADER_PATH);
    
    view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.
    
    program.SetProjectionMatrix(projection_matrix);
    program.SetViewMatrix(view_matrix);
    
    glUseProgram(program.programID);
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    // Existing
    state.player = new Entity();
    state.player->set_position(glm::vec3(0.0f, 4.0f, 0.0f));
    state.player->set_movement(glm::vec3(0.0f));
    state.player->speed = 1.0f;
    state.player->set_acceleration(glm::vec3(0.0f, -0.5, 0.0f));
    state.player->texture_id = load_texture(SPRITESHEET_FILEPATH);
    
    // Walking
    
    state.player->walking[state.player->LEFT]  = new int[1] {42};
    state.player->walking[state.player->RIGHT] = new int[1] {30};
    state.player->walking[state.player->DOWN]  = new int[1] {36};
    
    state.player->animation_indices = state.player->walking[state.player->DOWN];  // start George looking left
    state.player->animation_frames = 1;
    state.player->animation_index  = 0;
    state.player->animation_time   = 0.0f;
    state.player->animation_cols   = 6;
    state.player->animation_rows   = 12;
    state.player->set_height(0.80f);
    state.player->set_width(0.80f);
    
    // Platform stuff
   
    GLuint platform_texture_id = load_texture(PLATFORM_FILEPATH);
    
    state.platforms = new Entity[PLATFORM_COUNT];
    
    for (int j = 0; j < PLATFORM_COUNT - 1; j++)
    {
        
        state.platforms[j].texture_id = platform_texture_id;
        state.platforms[j].set_position(glm::vec3(j + -2.25f, -3.35f, 0.0f));
        state.platforms[j].update(0.0f, NULL, 0);
    }
    
    
    state.platforms[PLATFORM_COUNT - 1].texture_id = platform_texture_id;
    state.platforms[PLATFORM_COUNT - 1].set_position(glm::vec3(-4.55f, -3.35f, 0.0f));
    state.platforms[PLATFORM_COUNT - 1].update(0.0f, NULL, 0);
   
   
    // Pad
    GLuint pad_texture_id = load_texture(PAD_FILEPATH);
    state.pad = new Entity[PAD_COUNT];
    
    state.pad[PAD_COUNT - 1].texture_id = pad_texture_id;
    state.pad[PAD_COUNT - 1].set_position(glm::vec3(-3.335f, -3.35f, 0.0f));
    state.pad[PAD_COUNT - 1].update(0.0f, NULL, 0);
   
    for (int j = 0; j < PAD_COUNT - 1; j++)
    {
        
        state.pad[j].texture_id = pad_texture_id;
        state.pad[j].set_position(glm::vec3(j+-3.0f, j + -3.35f, 0.0f));
        state.pad[j].update(0.0f, NULL, 0);
    }
    
    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    state.player->set_movement(glm::vec3(0.0f));
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                game_is_running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        // Quit the game with a keystroke
                        game_is_running = false;
                        break;
                        
                    case SDLK_SPACE:
                        // Jump
                        if (state.player->collided_bottom) state.player->is_jumping = true;
                        break;
                      
                    case SDLK_LEFT:
                        state.player->set_acceleration(glm::vec3(-10.0f, 0.0f, 0.0f));
                        break;
                     
                        
                    case SDLK_RIGHT:
                        state.player->set_acceleration(glm::vec3(10.0f, 0.0f, 0.0f));
                        break;
                }
                
            default:
                break;
        }
        
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT])
    {
        state.player->movement.x = -1.0f;
        state.player->animation_indices = state.player->walking[state.player->LEFT];

        
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        state.player->movement.x = 1.0f;
        state.player->animation_indices = state.player->walking[state.player->RIGHT];
    }
    
    // This makes sure that the player can't move faster diagonally
    if (glm::length(state.player->movement) > 1.0f)
    {
        state.player->movement = glm::normalize(state.player->movement);
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - previous_ticks;

    previous_ticks = ticks;
    
    delta_time += accumulator;
    
    if (delta_time < FIXED_TIMESTEP)
    {
        accumulator = delta_time;
        return;
    }
    
    GLuint font_texture_id = load_texture(FONT_FILEPATH);
    
    // landing here indicates the player has lost
    for (int j = 0; j < PLATFORM_COUNT; ++j) {
        if (state.player->check_collision(&state.platforms[j])) {
            //state.player->DrawText(&program, font_texture_id, "YOU LOSE", WINDOW_WIDTH*WINDOW_HEIGHT, 1, glm::vec3(0.0f, 2.0f, 0.0f), FONTBANK_SIZE);
            state.player->speed = 0.0f;
            std::cout << "YOU LOSE!" << std::endl; // lose when landing on tan rocks
        }
    }
     
    // landing here indicates the player has one
    for (int i = 0; i < PAD_COUNT; ++i) {
        if (state.player->check_collision(&state.pad[i])) {
            std::cout << "YOU WIN!" << std::endl; // wins when landing on grey rocks
            state.player->speed = 0.0f;
            state.player->DrawText(&program, font_texture_id, "YOU WIN", WINDOW_WIDTH*WINDOW_HEIGHT, 1, glm::vec3(0.0f, 2.0f, 0.0f), FONTBANK_SIZE);
            //state.pad->DrawText(&program, font_texture_id, "YOU WIN", WINDOW_WIDTH*WINDOW_HEIGHT, 1, glm::vec3(0.0f, 3.0f, 0.0f), FONTBANK_SIZE);
        }
    }
    
    if (flag) {
     state.player->update(FIXED_TIMESTEP, state.platforms, PLATFORM_COUNT);
        flag = false;
    }
     
    else{
        state.player->update(FIXED_TIMESTEP, state.pad, PAD_COUNT);
        flag = true;
    }
    
    while (delta_time >= FIXED_TIMESTEP) {
        
        // Update. Notice it's FIXED_TIMESTEP. Not deltaTime
        
        delta_time -= FIXED_TIMESTEP;
    }
    accumulator = delta_time;
}


void render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    state.player->render(&program);
    //state.pad->render(&program);
    
    for (int i = 0; i < PAD_COUNT; i++) state.pad[i].render(&program);
    
    for (int i = 0; i < PLATFORM_COUNT; i++) state.platforms[i].render(&program);
    
    SDL_GL_SwapWindow(display_window);
}

void shutdown()
{
    SDL_Quit();
    
    delete [] state.platforms;
    delete [] state.pad;
    delete state.player;
}

/**
 DRIVER GAME LOOP
 */
int main(int argc, char* argv[])
{
    initialise();
    
    while (game_is_running)
    {
        process_input();
        update();
        render();
    }
    shutdown();
    return 0;
}
