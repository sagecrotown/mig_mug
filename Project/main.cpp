#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"
#include "Utility.h"
#include "Scene.h"
#include "Menu.h"
#include "LevelA.h"
#include "LevelB.h"
#include "LevelC.h"
#include "GameOver.h"
#include "GameWon.h"
#include "MurderWinner.h"
#include "Effects.h"


// ––––– CONSTANTS ––––– //
constexpr int WINDOW_WIDTH  = 640 * 2,
          WINDOW_HEIGHT = 480 * 2;

constexpr float BG_RED = 170.0/255.0,
                BG_BLUE    = 240.0/255.0,
                BG_GREEN   = 255.0/255.0,
                BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

constexpr char V_SHADER_PATH[]     =  "shaders/vertex_textured.glsl",
           F_SHADER_PATH[]         =  "shaders/fragment_textured.glsl";
constexpr char BGM_FILEPATH[]      =  "assets/mamples.mp3",
            DIGGING_SFX_FILEPATH[] =  "assets/dig_wet_sand.wav";

constexpr int NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL  = 0;
constexpr GLint TEXTURE_BORDER   = 0;

// BGM
constexpr int CD_QUAL_FREQ    = 44100,  // CD quality
          AUDIO_CHAN_AMT  = 2,      // Stereo
          AUDIO_BUFF_SIZE = 4096,
          LOOP_FOREVER   = -1;  // -1 means loop forever in Mix_PlayMusic; 0 means play once and loop zero times

// SFX
constexpr int PLAY_ONCE   =  0,
          NEXT_CHNL   = -1,  // next available channel
          MUTE_VOL    =  0,
          MILS_IN_SEC =  1000,
          ALL_SFX_CHN = -1;

// AUDIO
Mix_Music *g_music;
Mix_Chunk *g_digging_sfx;
bool muted = false;

// STRUCTS AND ENUMS
enum AppStatus { RUNNING, TERMINATED };
AppStatus g_app_status = RUNNING;

// ––––– GLOBAL VARIABLES ––––– //
Scene  *g_current_scene;
Menu *g_menu;
LevelA *g_levelA;
LevelB *g_levelB;
LevelC *g_levelC;
GameOver *g_gameOver;
GameWon *g_gameWon;
MurderWinner *g_murderWinner;

// borders
float left_edge;
float right_edge;
float bottom_edge;
float top_edge;

float view_left;
float view_right;
float view_bottom;
float view_top;
float view_x;
float view_y;

//Effects *g_effects;
Scene   *g_levels[7];

SDL_Window* g_display_window;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float delta_time;
float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

bool g_is_colliding_bottom = false;
std::vector<bool> preserved_colors;
int preserved_lives;

void swtich_to_scene(Scene *scene);
void initialise();
void soft_restart();
void process_input();
void update();
void render();
void shutdown();

// ––––– GENERAL FUNCTIONS ––––– //
void switch_to_scene(Scene *scene)
{
    if (g_current_scene == g_menu || g_current_scene == g_levelA || g_current_scene == g_levelB || g_current_scene == g_levelC) {
        preserved_colors = g_current_scene->get_colors();
        preserved_lives = g_current_scene->m_game_state.player->get_lives();
    }
    
    scene->set_colors(preserved_colors);
    
    g_current_scene = scene;
    g_current_scene->initialise(&g_shader_program);
    
    if (scene == g_levelA || scene == g_levelB || scene == g_levelC) {
        scene->m_game_state.player->set_lives(preserved_lives);
    }
    
    if (g_current_scene == g_gameWon || g_current_scene == g_gameOver || g_current_scene == g_murderWinner) {
        g_projection_matrix = glm::ortho(-15.0f, 15.0f, -11.25f, 11.25f, -1.0f, 1.0f);
        left_edge = 0.0f;
        top_edge = -2.0f;
        bottom_edge = -3.0f;
    }
    else {
        g_projection_matrix = glm::ortho(-10.0f, 10.0f, -7.5f, 7.5f, -1.0f, 1.0f);
        left_edge = 10.0f;
        bottom_edge = -72.0f;
        right_edge = 39.5f;
        top_edge = -7.0f;
    }
    g_shader_program.set_projection_matrix(g_projection_matrix);
}

void soft_restart()
{
    g_view_matrix = glm::mat4(1.0f);
    left_edge = 10.0f;
    bottom_edge = -12.0f;
    right_edge = 39.5f;
    top_edge = -7.0f;
    
    g_projection_matrix = glm::ortho(-10.0f, 10.0f, -7.5f, 7.5f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    g_menu = new Menu();
    g_levelA = new LevelA();
    g_levelB = new LevelB();
    g_levelC = new LevelC();
    g_gameOver = new GameOver();
    g_gameWon = new GameWon();
    g_murderWinner = new MurderWinner();
    
    g_levels[0] = g_menu;
    g_levels[1] = g_levelA;
    g_levels[2] = g_levelB;
    g_levels[3] = g_levelC;
    g_levels[4] = g_gameOver;
    g_levels[5] = g_gameWon;
    g_levels[6] = g_murderWinner;
    
    // Start at menu
    switch_to_scene(g_levels[0]);
    
//    g_effects = new Effects(g_projection_matrix, g_view_matrix);
//    g_effects->start(SHRINK, 2.0f);
    
    for (int i = 0; i < g_current_scene->m_game_state.colors.size(); i++) {
        g_current_scene->m_game_state.colors[i]->deactivate();
    }
    
    // Start Audio
    Mix_OpenAudio(
        CD_QUAL_FREQ,        // the frequency to playback audio at (in Hz)
        MIX_DEFAULT_FORMAT,  // audio format
        AUDIO_CHAN_AMT,      // number of channels (1 is mono, 2 is stereo, etc).
        AUDIO_BUFF_SIZE      // audio buffer size in sample FRAMES (total samples divided by channel count)
        );
    
    g_music = Mix_LoadMUS(BGM_FILEPATH);
    g_digging_sfx = Mix_LoadWAV(DIGGING_SFX_FILEPATH);
    
    if (!g_music) {
        std::cerr << "Error: Failed to load music! File path: " << BGM_FILEPATH << std::endl;
        std::cerr << "SDL_mixer Error: " << Mix_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }

    Mix_PlayMusic(g_music, LOOP_FOREVER);

    Mix_VolumeMusic(MIX_MAX_VOLUME );
    Mix_VolumeChunk(g_digging_sfx, MIX_MAX_VOLUME / 4 );
    
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Mig Mug",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    soft_restart();
}

void process_input() {
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    g_current_scene->get_state().player->set_movement(glm::vec3(0.0f));
    g_current_scene->get_state().player->face_forward();
    g_current_scene->get_state().player->set_not_digging();
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        // Quit the game with a keystroke
                        g_app_status = TERMINATED;
                        break;
                    
                    case SDLK_r: // restart the game
                        for (int i = 0; i < 3; i++) {
                            g_levels[i]->set_scene_id(-1);  // reset transitions
                        }
                        soft_restart(); // reinit
                        break;
                        
                    case SDLK_f:
                        break;
                        
                    case SDLK_m:
                        // Mute volume
                        if (!muted) {
                            Mix_VolumeMusic(MUTE_VOL);
                            Mix_VolumeChunk(g_digging_sfx, MUTE_VOL);
                            muted = true;
                        }
                        else {
                            Mix_VolumeMusic(MIX_MAX_VOLUME );
                            Mix_VolumeChunk(g_digging_sfx, MIX_MAX_VOLUME / 4 );
                            muted = false;
                        }
                        break;
                        
                    case SDLK_RETURN:
                        if (g_current_scene == g_menu) {
                            switch_to_scene(g_levels[1]);
                        }
                        break;
                    
                    case SDLK_SPACE:
                        if (g_current_scene->get_state().player->get_collided_bottom()) {
                            g_current_scene->get_state().player->jump();
                        }
                        
                    default:
                        break;
                }
                
            default:
                g_current_scene->get_state().player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
                break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT]) {
        g_current_scene->get_state().player->move_left();
    }
    else if (key_state[SDL_SCANCODE_RIGHT])  {
        g_current_scene->get_state().player->move_right();
    }
    
    if (key_state[SDL_SCANCODE_D])  {
        g_current_scene->get_state().player->dig();
        Mix_PlayChannel(
                        NEXT_CHNL,       // using the first channel that is not currently in use...
                        g_digging_sfx,  // ...play this chunk of audio...
                        PLAY_ONCE        // ...once.
                        );
    }
         
    if (glm::length( g_current_scene->get_state().player->get_movement()) > 1.0f)
        g_current_scene->get_state().player->normalise_movement();
    
}

void update() {
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    delta_time += g_accumulator;
    
    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }
    
    while (delta_time >= FIXED_TIMESTEP) {
        g_current_scene->update(FIXED_TIMESTEP);
//        g_effects->update(FIXED_TIMESTEP);
        
//        if (g_is_colliding_bottom == false && g_current_scene->get_state().player->get_collided_bottom()) g_effects->start(SHAKE, 1.0f);
        
        g_is_colliding_bottom = g_current_scene->get_state().player->get_collided_bottom();
        
        delta_time -= FIXED_TIMESTEP;
    }
    
    g_accumulator = delta_time;
    
    // Prevent the camera from showing anything outside of the "edge" of the level
    g_view_matrix = glm::mat4(1.0f);


    float player_x = g_current_scene->get_state().player->get_position().x;
    float player_y = g_current_scene->get_state().player->get_position().y;
    
    if (left_edge <= player_x && right_edge >= player_x) view_x = player_x;
    else if (left_edge > player_x) view_x = left_edge;
    else view_x = right_edge;
    
    if (top_edge >= player_y && bottom_edge <= player_y) view_y = player_y;
    else if (top_edge < player_y) view_y = top_edge;
    else view_y = bottom_edge;
    
    g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-view_x, -view_y, 0));
}

void render()
{
    g_shader_program.set_view_matrix(g_view_matrix);
       
    glClear(GL_COLOR_BUFFER_BIT);
       
    // ————— RENDERING THE SCENE (i.e. map, character, enemies...) ————— //
    g_current_scene->render(&g_shader_program);
       
//    g_effects->render();
    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();
    
    delete g_menu;
    delete g_levelA;
    delete g_levelB;
    delete g_levelC;
    delete g_gameOver;
    delete g_gameWon;
    delete g_murderWinner;
//    delete g_effects;
}

// ––––– DRIVER GAME LOOP ––––– //
int main(int argc, char* argv[])
{
    initialise();
    
    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        
        if (g_current_scene->get_state().next_scene_id >= 0) {
            int curr_fuel = g_current_scene->fuel_count;
            switch_to_scene(g_levels[g_current_scene->get_state().next_scene_id]);
            g_current_scene->fuel_count = curr_fuel;
        }
        
        render();
    }
    
    shutdown();
    return 0;
}
