#ifndef ENTITY_H
#define ENTITY_H

#include "Map.h"
#include "glm/glm.hpp"
#include "ShaderProgram.h"
#include <vector>
//#include "AI.h"

enum EntityType { PLATFORM, PLAYER, ENEMY, COLOR  };
enum AnimationDirection { FORWARD, LEFT, RIGHT, DIG, DIG_LEFT, DIG_RIGHT };

class Entity {
    
protected:
    bool m_is_active = true;
    bool m_is_digging = false;
    bool m_is_digging_left = false;
    bool m_is_digging_right = false;
    
    std::vector<std::vector<int>> m_frames; // vector of vectors containing indices of animation frames

    EntityType m_entity_type;
    
    // ————— TRANSFORMATIONS ————— //
    glm::vec3 m_movement;
    glm::vec3 m_position;
    glm::vec3 m_scale;
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;
    
    glm::vec3 m_start_pos;

    glm::mat4 m_model_matrix;

    float     m_speed,
              m_jumping_power,
              m_angle = 0;
    
    bool m_is_jumping;

    // ————— TEXTURES ————— //
    GLuint    m_texture_id;

    // ————— ANIMATION ————— //
    int m_animation_cols;
    int m_current_frames,
        m_animation_index,
        m_animation_rows;

    std::vector<int> m_animation_indices;
    float m_animation_time = 0.0f;

    float m_width = 1.0f,
          m_height = 1.0f;
    
    // ————— COLLISIONS ————— //
    bool m_collided_top    = false;
    bool m_collided_bottom = false;
    bool m_collided_left   = false;
    bool m_collided_right  = false;
    bool m_on_triangle = false;
//    bool m_down_is_diggable = false;
//    bool m_left_is_diggable = false;
//    bool m_right_is_diggable = false;
    
    // ————— GAME LOGIC ————— //
    bool m_level_won = false;
    int m_lives = 2;

public:
    // ————— STATIC VARIABLES ————— //
    static constexpr int FRAMES_PER_SECOND = 6;

    // ————— METHODS ————— //
    
    Entity();
    Entity(GLuint texture_id, float speed, glm::vec3 acceleration, float jump_power, std::vector<std::vector<int>> frames, float animation_time, int current_frames, int animation_index, int animation_cols, int animation_rows, float width, float height, EntityType EntityType);
    Entity(GLuint texture_id, float speed, float width, float height, EntityType EntityType); // Simpler constructor
    ~Entity();

    void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index);
    bool const check_collision(Entity* other) const;
    
    void const check_collision_x(std::vector<Entity*> collidable_entities, int collidable_entity_count);
    void const check_collision_y(std::vector<Entity*> collidable_entities, int collidable_entity_count, std::vector<Entity*> colors);
    
    // Overloading our methods to check for only the map
    void const check_collision_y(Map *map);
    void const check_collision_x(Map *map);
    
    virtual void update(float delta_time, Entity *player, std::vector<Entity*> collidable_entities, int collidable_entity_count, std::vector<Entity*> colors, Map *map);
    void render(ShaderProgram* program);
    
    void normalise_movement() { m_movement = glm::normalize(m_movement); }

    virtual void face_left() { m_animation_indices = m_frames[LEFT]; }
    virtual void face_right() { m_animation_indices = m_frames[RIGHT]; }
    void face_forward() { m_animation_indices = m_frames[FORWARD]; }
    void dig_down() { m_animation_indices = m_frames[DIG]; }
    void dig_left() { m_animation_indices = m_frames[DIG_LEFT]; }
    void dig_right() { m_animation_indices = m_frames[DIG_RIGHT]; }

    void move_left() { m_movement.x = -1.0f; face_left(); }
    void move_right() { m_movement.x = 1.0f;  face_right(); }
    void move_up() { m_movement.y = 1.0f;  face_forward(); }
    void move_down() { m_movement.y = -1.0f; face_forward(); }
    
    void dig();
    
    void const jump() { m_is_jumping = true;}

    // ————— GETTERS ————— //
    EntityType const get_entity_type()    const { return m_entity_type;   };
    float const get_jumping_power() const { return m_jumping_power; }
    glm::vec3 const get_position()     const { return m_position; }
    glm::vec3 const get_velocity()     const { return m_velocity; }
    glm::vec3 const get_acceleration() const { return m_acceleration; }
    glm::vec3 const get_movement()     const { return m_movement; }
    glm::vec3 const get_scale()        const { return m_scale; }
    GLuint    const get_texture_id()   const { return m_texture_id; }
    float     const get_speed()        const { return m_speed; }
    bool      const get_collided_top() const { return m_collided_top; }
    bool      const get_collided_bottom() const { return m_collided_bottom; }
    bool      const get_collided_right() const { return m_collided_right; }
    bool      const get_collided_left() const { return m_collided_left; }
    bool      const get_on_triangle() const { return m_on_triangle; }
    int       const get_animation_index() const { return m_animation_index; }
    int       const get_lives() const { return m_lives; }
    bool      const is_active() const { return m_is_active; }
    bool      const is_digging() const { return m_is_digging; }
    bool      const is_digging_left() const { return m_is_digging_left; }
    bool      const is_digging_right() const { return m_is_digging_right; }
    bool      const level_won() const { return m_level_won; }
    
    void activate()   { m_is_active = true;  };
    void deactivate() { m_is_active = false; };
    
    // ————— SETTERS ————— //
    void const set_entity_type(EntityType new_entity_type)  { m_entity_type = new_entity_type;}
    virtual void const set_position(glm::vec3 new_position) { m_position = new_position; }
    void const set_velocity(glm::vec3 new_velocity) { m_velocity = new_velocity; }
    void const set_acceleration(glm::vec3 new_acceleration) { m_acceleration = new_acceleration; }
    void const set_movement(glm::vec3 new_movement) { m_movement = new_movement; }
    void const set_scale(glm::vec3 new_scale) { m_scale = new_scale; }
    void const set_texture_id(GLuint new_texture_id) { m_texture_id = new_texture_id; }
    void const set_speed(float new_speed) { m_speed = new_speed; }
    void const set_animation_cols(int new_cols) { m_animation_cols = new_cols; }
    void const set_animation_rows(int new_rows) { m_animation_rows = new_rows; }
    void const set_current_frames(int new_frames) { m_current_frames = new_frames; }
    void const set_animation_index(int new_index) { m_animation_index = new_index; }
    void const set_animation_time(float new_time) { m_animation_time = new_time; }
    void const set_jumping_power(float new_jumping_power) { m_jumping_power = new_jumping_power;}
    void const set_width(float new_width) {m_width = new_width; }
    void const set_height(float new_height) {m_height = new_height; }
    void const set_lives(int new_lives) { m_lives = new_lives; }
    void const set_start_pos(glm::vec3 start_pos) { m_start_pos = start_pos; }
    void const change_angle(float difference) {m_angle = m_angle + difference; }
    void const set_not_digging() { m_is_digging = false; m_is_digging_left = false; m_is_digging_right = false; }

    // Setter for m_frames
    void set_frames(std::vector<std::vector<int>> frames) {
        for (int i = 0; i < frames.size(); i++) {
            for (int j = 0; j < frames[i].size(); j++) {
                m_frames[i].push_back(frames[i][j]);
            }
        }
    }
};

#endif // ENTITY_H
