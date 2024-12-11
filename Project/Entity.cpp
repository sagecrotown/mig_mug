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
#include "Entity.h"
#include "AI.h"
#include <cmath>


// Default constructor
Entity::Entity()
    : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
    m_speed(0.0f), m_animation_cols(0), m_current_frames(0), m_animation_index(0),
    m_animation_rows(0),  m_animation_time(0.0f),
m_texture_id(0), m_velocity(0.0f), m_acceleration(0.0f), m_width(0.0f), m_height(0.0f), m_frames(FRAMES_PER_SECOND), m_is_active(true), m_lives(2)
{
    // Initialize m_frames with zeros or any default value
    for (int i = 0; i < FRAMES_PER_SECOND; ++i) {
        for (int j = 0; j < FRAMES_PER_SECOND; ++j) {
            m_frames[i].push_back(0);
        }
    }
}

// Parameterized constructor
Entity::Entity(GLuint texture_id, float speed, glm::vec3 acceleration, float jump_power, std::vector<std::vector<int>> frames, float animation_time, int current_frames, int animation_index, int animation_cols,
    int animation_rows, float width, float height, EntityType EntityType)
    : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f), m_speed(speed),m_acceleration(acceleration), m_jumping_power(jump_power), m_animation_cols(animation_cols), m_current_frames(current_frames), m_animation_index(animation_index), m_animation_rows(animation_rows), m_animation_time(animation_time), m_texture_id(texture_id), m_velocity(0.0f), m_width(width), m_height(height), m_entity_type(EntityType), m_frames(frames.size(), std::vector<int> (0)),  m_is_active(true), m_lives(2)
{
    set_frames(frames);
    face_forward();
}

// Simpler constructor for partial initialization
Entity::Entity(GLuint texture_id, float speed,  float width, float height, EntityType EntityType)
    : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f), m_speed(speed), m_animation_cols(0), m_current_frames(0), m_animation_index(0), m_animation_rows(0), m_animation_time(0.0f), m_texture_id(texture_id), m_velocity(0.0f), m_acceleration(0.0f), m_width(width), m_height(height), m_entity_type(EntityType), m_frames(FRAMES_PER_SECOND, std::vector<int> (FRAMES_PER_SECOND, 0)), m_is_active(true), m_lives(2) {}

Entity::~Entity() { }

void Entity::dig() {
    m_is_digging = true;
    if (m_movement.x < 0) {
        m_movement.x = 0;
        dig_left();
        m_is_digging_left = true;
    }
    else if (m_movement.x > 0) {
        m_movement.x = 0;
        dig_right();
        m_is_digging_right = true;
    }
    else
        dig_down();
}

void Entity::draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index) {
    // Step 1: Calculate the UV location of the indexed frame
    float u_coord = (float)(index % m_animation_cols) / (float)m_animation_cols;
    float v_coord = (float)(index / m_animation_cols) / (float)m_animation_rows;

    // Step 2: Calculate its UV size
    float width = 1.0f / (float)m_animation_cols;
    float height = 1.0f / (float)m_animation_rows;

    // Step 3: Just as we have done before, match the texture coordinates to the vertices
    float tex_coords[] = {
        u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
        u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
    };

    float vertices[] = {
        -0.5, -0.5, 0.5, -0.5,  0.5, 0.5,
        -0.5, -0.5, 0.5,  0.5, -0.5, 0.5
    };

    // Step 4: And render
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

bool const Entity::check_collision(Entity* other) const {
    float x_distance = fabs(m_position.x - other->m_position.x) - ((m_width + other->m_width) / 2.0f);
    float y_distance = fabs(m_position.y - other->m_position.y) - ((m_height + other->m_height) / 2.0f);

    return x_distance < 0.0f && y_distance < 0.0f && other->m_is_active;
}

void const Entity::check_collision_y(std::vector<Entity*> collidable_entities, int collidable_entity_count, std::vector<Entity*> colors) {
    for (int i = 0; i < collidable_entity_count; i++) {
        Entity *collidable_entity = collidable_entities[i];
        
        if (check_collision(collidable_entity)) {
            float y_distance = fabs(m_position.y - collidable_entity->m_position.y);
            float y_overlap = fabs(y_distance - (m_height / 2.0f) - (collidable_entity->m_height / 2.0f));
            if (m_velocity.y > 0) {
                m_position.y   -= y_overlap;
                m_velocity.y    = 0;

                // Collision!
                m_collided_top  = true;
            }
            else if (m_velocity.y < 0) {
                m_position.y      += y_overlap;
                m_velocity.y       = 0;

                // Collision!
//                m_collided_bottom  = true;
                AI* aiPtr = dynamic_cast<AI*>(collidable_entity);     // cast to AI, if possible
                if (get_entity_type() == PLAYER && aiPtr) {        // if you are player and other is AI
                    aiPtr->deactivate();
                    colors[aiPtr->get_ai_type()]->activate();
                }
            }
            
            if(collidable_entity->get_entity_type() == PLATFORM) {        // if other is target
                m_level_won = true;
            }
        }
    }
}

void const Entity::check_collision_x(std::vector<Entity*> collidable_entities, int collidable_entity_count) {
    for (int i = 0; i < collidable_entity_count; i++) {
        Entity *collidable_entity = collidable_entities[i];
        
        if (check_collision(collidable_entity)) {
            float x_distance = fabs(m_position.x - collidable_entity->m_position.x);
            float x_overlap = fabs(x_distance - (m_width / 2.0f) - (collidable_entity->m_width / 2.0f));
            
            if (m_velocity.x > 0 || collidable_entity->get_velocity().x < 0 ) {
                m_position.x     -= x_overlap;
                m_velocity.x      = 0;
                
//                m_collided_right  = true;
                if (dynamic_cast<AI*>(collidable_entity)) {  // if you crashed into an enemy
                    if (m_lives == 0 ) {
                        deactivate();   // dead
                    }
                    else m_lives--;
                    set_position(m_start_pos);
                }
                
            }
            else if (m_velocity.x < 0 || collidable_entity->get_velocity().x > 0 ) {
                m_position.x    += x_overlap;
                m_velocity.x     = 0;
 
//                m_collided_left  = true;
                if (dynamic_cast<AI*>(collidable_entity)) {  // if you crashed into an enemy
                    if (m_lives == 0 ) {
                        deactivate();   // dead
                    }
                    else m_lives--;
                    set_position(m_start_pos);
                }
            }
        }
    }
}

void const Entity::check_collision_y(Map *map) {
    // Probes for tiles above
    glm::vec3 top = glm::vec3(m_position.x, m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_left = glm::vec3(m_position.x - (m_width / 2), m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_right = glm::vec3(m_position.x + (m_width / 2), m_position.y + (m_height / 2), m_position.z);
    
    // Probes for tiles below
    glm::vec3 bottom = glm::vec3(m_position.x, m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_left = glm::vec3(m_position.x - (m_width / 2), m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_right = glm::vec3(m_position.x + (m_width / 2), m_position.y - (m_height / 2), m_position.z);
    
    float penetration_x = 0;
    float penetration_y = 0;
    
    // If the map is solid, check the top three points
    if (map->is_solid(top, &penetration_x, &penetration_y) && m_velocity.y > 0) {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }
    else if (map->is_solid(top_left, &penetration_x, &penetration_y) && m_velocity.y > 0) {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }
    else if (map->is_solid(top_right, &penetration_x, &penetration_y) && m_velocity.y > 0) {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }
    
    // And the bottom three points
    if (map->is_solid(bottom, &penetration_x, &penetration_y) && m_velocity.y < 0) {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;
        if (map->is_triangle(bottom)) {
            m_on_triangle = true;
        }
    }
    else if (map->is_solid(bottom_left, &penetration_x, &penetration_y) && m_velocity.y < 0) {
            m_position.y += penetration_y;
            m_velocity.y = 0;
            m_collided_bottom = true;
            if (map->is_triangle(bottom_left)) {
                m_on_triangle = true;
            }
    }
    else if (map->is_solid(bottom_right, &penetration_x, &penetration_y) && m_velocity.y < 0) {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;
        if (map->is_triangle(bottom_right)) {
            m_on_triangle = true;
        }
        
    }
}

void const Entity::check_collision_x(Map *map) {
    // Probes for tiles; the x-checking is much simpler
    glm::vec3 left  = glm::vec3(m_position.x - (m_width / 2), m_position.y, m_position.z);
    glm::vec3 right = glm::vec3(m_position.x + (m_width / 2), m_position.y, m_position.z);
    
    float penetration_x = 0;
    float penetration_y = 0;
    
    if (map->is_solid(left, &penetration_x, &penetration_y) && m_velocity.x < 0)
    {
        m_position.x += penetration_x;
        m_velocity.x = 0;
        m_collided_left = true;
    }
    if (map->is_solid(right, &penetration_x, &penetration_y) && m_velocity.x > 0)
    {
        m_position.x -= penetration_x;
        m_velocity.x = 0;
        m_collided_right = true;
    }
}
void Entity::update(float delta_time, Entity *player, std::vector<Entity*> collidable_entities, int collidable_entity_count, std::vector<Entity*> colors, Map *map) {
    if (!m_is_active) return;
    
    if (get_entity_type() == COLOR) {
        
        if (player->get_movement().x < 0) {    // match animation state
            face_left();
        }
        else if (player->get_movement().x > 0) {
            face_right();
        }
        else face_forward();
        
        set_position(player->get_position());  // match position
        set_animation_index(player->get_animation_index());  // make sure indices are lined up
        
        m_model_matrix = glm::mat4(1.0f);
        m_model_matrix = glm::translate(m_model_matrix, m_position);
        m_model_matrix = glm::rotate(m_model_matrix, m_angle, glm::vec3(0.0f, 0.0f, 1.0f));
        m_model_matrix = glm::scale(m_model_matrix, m_scale);
    
        return;
    }
 
    m_collided_top    = false;
    m_collided_bottom = false;
    m_collided_left   = false;
    m_collided_right  = false;
    m_on_triangle = false;
    
//    m_is_digging = false;
//    m_is_digging_left = false;
//    m_is_digging_right = false;
    
    if (m_animation_indices.size() != 0) {

        m_animation_time += delta_time;
        float seconds_per_frame = (float) 1 / FRAMES_PER_SECOND;
        
        if (m_animation_time >= seconds_per_frame) {
            m_animation_time = 0.0f;
            m_animation_index++;
            
            if (m_animation_index >= m_animation_indices.size()) {
                m_animation_index = 0;
            }
        }
    }

    
    if (m_is_jumping) {  // find acceleration components based on current angle of rotation
        m_is_jumping = false;
        m_velocity.y += m_jumping_power;
    }
    
    m_velocity.x += m_acceleration.x * delta_time;
    m_velocity.y += m_acceleration.y * delta_time;
    
    m_velocity.x = m_movement.x * m_speed;

    m_position.y += m_velocity.y * delta_time;
    m_position.x += m_velocity.x * delta_time;
    
    check_collision_y(map);
    check_collision_y(collidable_entities, collidable_entity_count, colors);
    
    check_collision_x(collidable_entities, collidable_entity_count);
    check_collision_x(map);
    
    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);
    m_model_matrix = glm::rotate(m_model_matrix, m_angle, glm::vec3(0.0f, 0.0f, 1.0f));
    m_model_matrix = glm::scale(m_model_matrix, m_scale);
}


void Entity::render(ShaderProgram* program) {
    
    if (!m_is_active) return;
    
    program->set_model_matrix(m_model_matrix);

    if (m_animation_indices.size() != 0) {
        if (m_animation_index >= m_animation_indices.size()) m_animation_index = 0;
        
        draw_sprite_from_texture_atlas(program, m_texture_id, m_animation_indices[m_animation_index]);
        return;
    }

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float tex_coords[] = { 0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };

    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}
