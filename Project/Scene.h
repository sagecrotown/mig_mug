#pragma once
#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <vector>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Util.h"
#include "Entity.h"
#include "AI.h"
#include "Map.h"
#include "Utility.h"

struct GameState
{
    // ————— GAME OBJECTS ————— //
    Map *map;
    Entity *player;
    Entity *target;
    
    std::vector<int> LEVEL_DATA;
    std::vector<Entity*> collidables;
    std::vector<AI*> enemies;
    std::vector<Entity*> colors;
    std::vector<bool> active_colors;
    
    // ————— AUDIO ————— //
    Mix_Music *bgm;
    Mix_Chunk *jump_sfx;
    
    // ————— POINTERS TO OTHER SCENES ————— //
    int next_scene_id;
};

class Scene {
    
public:
    // ————— ATTRIBUTES ————— //
    int ENEMY_COUNT = 1;
    int fuel_count = 0;
    GLuint font_texture_id;
    
    // ————— GAME/SCREEN VARIABLES ————— //
    float left_edge = 10.0f;
    float bottom_edge = -12.0f;
    float right_edge = 39.5f;
    float top_edge = -7.0f;
    
    float view_x,
          view_y;
    
    GameState m_game_state;
    
    // ————— METHODS ————— //
    virtual ~Scene() = default;
    
    virtual void initialise(ShaderProgram *program) = 0;
    virtual void update(float delta_time) = 0;
    virtual void render(ShaderProgram *program) = 0;
    
//    virtual void flash_text(ShaderProgram *program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position, float duration);
    
    // ————— GETTERS ————— //
    GameState const get_state() const { return m_game_state;             }
    int const get_enemy_count() const { return ENEMY_COUNT; }
    
    void set_scene_id(int id) { m_game_state.next_scene_id = id; }
    void set_colors(std::vector<bool> new_colors) {m_game_state.active_colors = new_colors; }
    std::vector<bool> get_colors() { return m_game_state.active_colors; }
};
