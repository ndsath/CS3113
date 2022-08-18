
#include "MenuScreen.h"
#include "Utility.h"
const char TEXT_PATH[] = "font1.png";
GLuint text_texture_id_screen;
#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8


MenuScreen::~MenuScreen()
{
    delete [] this->state.power;
    delete [] this->state.enemies;
    delete    this->state.player;
    delete    this->state.map;
    Mix_FreeChunk(this->state.jump_sfx);
    Mix_FreeMusic(this->state.bgm);
}

void MenuScreen::initialise()
{
    state.next_scene_id = -1;
    text_texture_id_screen = Utility::load_texture(TEXT_PATH);
    
    /**
     BGM and SFX
     */

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    state.bgm = Mix_LoadMUS("dooblydoo.mp3");
    Mix_PlayMusic(state.bgm, -1);
    Mix_VolumeMusic(0.0f);
    
    state.jump_sfx = Mix_LoadWAV("bounce.wav");
}

void MenuScreen::update(float delta_time) {
    
}


void MenuScreen::render(ShaderProgram *program)
{
    Utility::draw_text(program, text_texture_id_screen, "Knights vs Aliens: ", 0.49f, 0.005f, glm::vec3(1.0f, -2.0f, 0.0f));
    Utility::draw_text(program, text_texture_id_screen, "The Final Version", 0.42f, 0.005f, glm::vec3(1.5f, -4.0f, 0.0f));
}

