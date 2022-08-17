#include "LevelA.h"
#include "Utility.h"
#define FIXED_TIMESTEP 0.0166666f

bool loss_flag = false;
bool win_flag = false;
GLuint text_texture_id;
const char TEXT_PATH[] = "font1.png";
#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

unsigned int LEVELA_DATA[] =
{
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

LevelA::~LevelA()
{
    delete [] this->state.enemies;
    delete    this->state.player;
    delete    this->state.map;
    Mix_FreeChunk(this->state.jump_sfx);
    Mix_FreeMusic(this->state.bgm);
}

void LevelA::initialise()
{
    state.next_scene_id = -1;
    
    GLuint map_texture_id = Utility::load_texture("brick.png");
    this->state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELA_DATA, map_texture_id, 1.0f, 4, 1);
    
    
    // Code from main.cpp's initialise()
    /**
     George's Stuff
     */
    state.player = new Entity();
    state.player->set_entity_type(PLAYER);
    state.player->set_position(glm::vec3(5.0f, 0.0f, 0.0f));
    state.player->set_movement(glm::vec3(0.0f));
    state.player->speed = 2.5f;
    state.player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    state.player->texture_id = Utility::load_texture("knight.png");
    
    // Walking
    state.player->walking[state.player->LEFT]  = new int[3] {86, 87, 88};
    //state.player->walking[state.player->LEFT]  = new int[9] {80, 81, 82, 83, 84, 85, 86, 87, 88};
    //state.player->walking[state.player->RIGHT] = new int[9] {96, 97, 98, 99, 100, 101, 102, 103, 104};
    state.player->walking[state.player->RIGHT] = new int[3] {99, 100, 101};
    //state.player->walking[state.player->UP]    = new int[4] { 2, 6, 10, 14 };
    //state.player->walking[state.player->DOWN]  = new int[4] { 0, 4, 8,  12 };

    state.player->animation_indices = state.player->walking[state.player->RIGHT];  // start George looking left
    state.player->animation_frames = 3;
    state.player->animation_index  = 0;
    state.player->animation_time   = 0.0f;
    state.player->animation_cols   = 9;
    state.player->animation_rows   = 16;
    state.player->set_height(0.8f);
    state.player->set_width(0.8f);
    
    // Jumping
    state.player->jumping_power = 5.0f;
    
    
    GLuint enemy_texture_id = Utility::load_texture("evil.png");
    
    state.enemies = new Entity[this->ENEMY_COUNT];
    state.enemies[0].set_entity_type(ENEMY);
    state.enemies[0].set_ai_type(GUARD);
    state.enemies[0].set_ai_state(IDLE);
    state.enemies[0].texture_id = enemy_texture_id;
    state.enemies[0].set_position(glm::vec3(2.0f, 0.0f, 0.0f));
    state.enemies[0].set_movement(glm::vec3(0.0f));
    state.enemies[0].speed = 1.0f;
    state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    state.enemies->set_height(0.8f);
    state.enemies->set_width(0.8f);
    text_texture_id = Utility::load_texture(TEXT_PATH);
    
    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    state.bgm = Mix_LoadMUS("dooblydoo.mp3");
    Mix_PlayMusic(state.bgm, -1);
    Mix_VolumeMusic(0.0f);
    
    state.jump_sfx = Mix_LoadWAV("bounce.wav");
}

void LevelA::update(float delta_time)
{
    this->state.player->update(delta_time, state.player, state.enemies, this->ENEMY_COUNT, this->state.map);
    this->state.enemies->update(delta_time, state.player, state.player, this->ENEMY_COUNT, this->state.map);
    
    if (this->state.player->get_position().y < -10.0f) state.next_scene_id = 1;
    if (this->state.player->collided_with_enemy_bottom) {
        win_flag = true;
        loss_flag = false;
    }
    
     if ((this->state.player->collided_with_enemy_right || this->state.enemies->collided_with_player_left)||
         (this->state.player->collided_with_enemy_left || this->state.enemies->collided_with_player_right )) {
         loss_flag= true;
         win_flag = false;
    }
}

void LevelA::render(ShaderProgram *program)
{
    this->state.player->render(program);
    this->state.map->render(program);
    
    if (!win_flag) {
        this->state.enemies->render(program);
    }
    
    if (loss_flag) {
        Utility::draw_text(program, text_texture_id, "YOU LOSE!", 1.0f, 0.005f, glm::vec3(1.0f, -5.0f, 0.0f));
    }
    
    if (win_flag) {
        Utility::draw_text(program, text_texture_id, "ALIEN 1 DEAD!", 0.65f, 0.005f, glm::vec3(1.0f, -5.0f, 0.0f));
        //Utility::draw_text(program, text_texture_id, "MOVE TO THE RIGHT FOR LEVEL 2", 0.5f, 0.005f, glm::vec3(1.0f, -6.0f, 0.0f));

    }
}

