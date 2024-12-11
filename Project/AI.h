#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#include <cmath>

#include "Entity.h"

enum AIType     { BLUE, RED, YELLOW, GREEN, PURPLE };
enum AIState    { WALKING, IDLE, SQUASHED };

class AI : public Entity {
    
private:
   
    AIType     m_ai_type;
    AIState    m_ai_state;
    bool       moving_right;
    bool       moving_up;

public:

    AI();
    AI(GLuint texture_id, float speed, glm::vec3 acceleration, float jump_power, std::vector<std::vector<int>> frames, float animation_time, int current_frames, int animation_index, int animation_cols, int animation_rows, float width, float height, EntityType EntityType, AIType AIType, AIState AIState);
    AI(GLuint texture_id, float speed, float width, float height, EntityType EntityType, AIType AIType, AIState AIState); // AI constructor
    ~AI();

    void update(float delta_time, Entity *player, int collidable_entity_count, Map *map);
    
    void ai_activate(Entity *player);
    void ai_walk(int left_bound, int right_bound);
    void ai_guard(Entity *player);
    void ai_fly(int lower_bound, int upper_bound);
    
    // TODO: break into two enums: color and state/direction. should be able to specify type, and from type have several animation frame sets to work with.
    void look_blue() { m_animation_indices = m_frames[BLUE]; }
    void look_red() { m_animation_indices = m_frames[RED]; }
    void look_yellow() { m_animation_indices = m_frames[YELLOW]; }
    void look_green() { m_animation_indices = m_frames[GREEN]; }
    void look_purple() { m_animation_indices = m_frames[PURPLE + 1]; } // +1 bc green has two animation sets
    
    void face_left() {
        if(get_ai_type() == GREEN) {
            look_green();
        }
        else if (get_ai_type() == PURPLE) {
            look_purple();
        }
    }
    
    void face_right() {
        if(get_ai_type() == GREEN) {
            m_animation_indices = m_frames[GREEN + 1];
        }
        else if (get_ai_type() == PURPLE) {
            m_animation_indices = m_frames[PURPLE + 2];
        }
    }
//    void move_left() { m_movement.x = -1.0f; face_left(); }
//    void move_right() { m_movement.x = 1.0f;  face_right(); }
//    void move_up() { m_movement.y = 1.0f;  face_forward(); }
//    void move_down() { m_movement.y = -1.0f; face_forward(); }
    
    // ————— GETTERS ————— //
    AIType     const get_ai_type()        const { return m_ai_type; };
    AIState    const get_ai_state()       const { return m_ai_state; };
    
    // ————— SETTERS ————— //
    void const set_ai_type(AIType new_ai_type);
    void const set_ai_state(AIState new_state);
};

