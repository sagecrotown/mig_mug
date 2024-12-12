#include "AI.h"

// Default constructor
AI::AI() : Entity::Entity(), m_ai_type(BLUE), m_ai_state(IDLE) {}  // Call Entity constructor

AI::AI(GLuint texture_id, float speed, glm::vec3 acceleration, float jump_power, std::vector<std::vector<int>> frames, float animation_time, int current_frames, int animation_index, int animation_cols, int animation_rows, float width, float height, EntityType EntityType, AIType AIType, AIState AIState) :
Entity::Entity(texture_id, speed, acceleration, jump_power, frames, animation_time, current_frames, animation_index, animation_cols, animation_rows, width, height, EntityType), m_ai_type(AIType), m_ai_state(AIState) {}

AI::AI(GLuint texture_id, float speed, float width, float height, EntityType EntityType, AIType AIType, AIState AIState) :
Entity::Entity(texture_id, speed, width, height, EntityType), m_ai_type(AIType), m_ai_state(AIState) {}

AI::~AI() { }

void AI::update(float delta_time, Entity *player, int collidable_entity_count, Map *map) {
    if (!m_is_active) return;
 
    m_collided_top    = false;
    m_collided_bottom = false;
    m_collided_left   = false;
    m_collided_right  = false;
    m_on_triangle = false;
    
    ai_activate(player);
    
    if (m_animation_indices.size() != 0)
    {
        m_animation_time += delta_time;
        float seconds_per_frame = (float) 1 / FRAMES_PER_SECOND;
        
        if (m_animation_time >= seconds_per_frame)
        {
            m_animation_time = 0.0f;
            m_animation_index++;
            
            if (m_animation_index >= m_current_frames)
            {
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
    check_collision_x(map);
    
    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);
    m_model_matrix = glm::rotate(m_model_matrix, m_angle, glm::vec3(0.0f, 0.0f, 1.0f));
    m_model_matrix = glm::scale(m_model_matrix, m_scale);
}

void AI::ai_activate(Entity *player) {
    switch (m_ai_type) {
            
        case PURPLE:        // purple + green can both walk
            look_purple();
            ai_guard(player);
            break;
            
        case GREEN:
            look_green();
            ai_guard(player);
            break;
        
        case RED:          // red + yellow + blue are the same
            look_red();
            ai_fly(m_fly_start, m_fly_start + m_fly_range);
            break;
            
        case YELLOW:
            look_yellow();
            break;
            
        case BLUE:
            look_blue();
            ai_walk(m_shuffle_start, m_shuffle_start + m_shuffle_range);
            break;
            
        default:
            break;
    }
}

void AI::ai_walk(int left_bound, int right_bound) {
    
    if (m_position.x > (right_bound)) {
        moving_right = false;
    }
    
    if (m_position.x < (left_bound)) {
        moving_right = true;
    }
    
    if (moving_right) {
        m_movement = glm::vec3(1.0f, 0.0f, 0.0f);
    }
    else {
        m_movement = glm::vec3(-1.0f, 0.0f, 0.0f);
    }
}

void AI::ai_fly(int lower_bound, int upper_bound) {
    if (m_position.y >= (upper_bound)) {
        moving_up = false;
    }
    
    if (m_position.y <= (lower_bound)) {
        moving_up = true;
    }
    
    if (moving_up) {
        m_movement = glm::vec3(0.0f, 1.0f, 0.0f);
    }
    else {
        m_movement = glm::vec3(0.0f, -1.0f, 0.0f);
    }
    
    m_velocity.y = m_movement.y * m_speed;
}

void AI::ai_guard(Entity *player) {
    switch (m_ai_state) {
        case IDLE:
            if (glm::distance(m_position, player->get_position()) < 12.0f) m_ai_state = WALKING;
            break;
            
        case WALKING:
            if (m_position.x > player->get_position().x) {
                m_movement = glm::vec3(-1.0f, 0.0f, 0.0f);
                face_left();
            } else {
                m_movement = glm::vec3(1.0f, 0.0f, 0.0f);
                face_right();
            }
            if (fabs(m_position.x - player->get_position().x) < 0.05f) {
                face_left();
            }
            break;
            
//        case SQUASHED:
//            break;
            
        default:
            break;
    }
}

void const AI::set_ai_type(AIType new_ai_type) {
    m_ai_type = new_ai_type;
}

void const AI::set_ai_state(AIState new_state) {
    m_ai_state = new_state;
}

void const AI::set_position(glm::vec3 new_position, int fly_range, int shuffle_range) {
    m_position = new_position;
    m_fly_start = get_position().y;
    m_shuffle_start = get_position().x;
    m_fly_range = fly_range;
    m_shuffle_range = shuffle_range;
}

    

