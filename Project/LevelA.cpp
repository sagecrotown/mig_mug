#include "LevelA.h"

#define LEVEL_WIDTH 50
#define LEVEL_HEIGHT 80

constexpr char SPRITESHEET_FILEPATH[] = "assets/sage_miner.png",
           ENEMY_FILEPATH[]       = "assets/aiiiii.png",
            MAP_FILEPATH[] = "assets/mars_tile.png",
          FONT_FILEPATH[] = "assets/blue_font.png",
            CSV_FILEPATH[] = "assets/final_1.csv",
            TARGET_FILEPATH[] = "assets/target.png";

constexpr char* COLOR_FILEPATHS[] = {
    "assets/blue_spot.png",
    "assets/red_spot.png",
    "assets/yellow_spot.png",
    "assets/green_spot.png",
    "assets/purple_spot.png",
};

LevelA::~LevelA() {
    for (int i = 0; i < m_game_state.collidables.size(); i++) {
        delete m_game_state.collidables[i];
    }
    delete    m_game_state.player;
    delete    m_game_state.map;
//    Mix_FreeChunk(m_game_state.jump_sfx);
//    Mix_FreeMusic(m_game_state.bgm);
}

void LevelA::initialise(ShaderProgram *program) {

    m_game_state.next_scene_id = -1;
    m_game_state.LEVEL_DATA.resize(LEVEL_WIDTH * LEVEL_HEIGHT);
    
    // TODO: convert level data to vector so it can by dynamically allocated
    
    Utility::readCSV(CSV_FILEPATH, m_game_state.LEVEL_DATA, LEVEL_WIDTH * LEVEL_HEIGHT);
    
    GLuint map_texture_id = Utility::load_texture(MAP_FILEPATH);
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, m_game_state.LEVEL_DATA, map_texture_id, 1.0f, 7, 3);
    
    font_texture_id = Utility::load_texture(FONT_FILEPATH);
    ENEMY_COUNT = 22;  // write to scene enemy counter?
    
    std::vector<std::vector<int>> player_animation = {
        { 0 },               // facing forward
        { 1 , 2 , 3 , 4 },   // walking left
        { 5 , 6 , 7 , 8 },   // walking right
        { 9 , 10 },           // digging down
        { 11, 12 },           // digging left
        { 13, 14 }            // digging right
    };
    
//    std::vector<std::vector<int>> color_animation = {
//        { 0 , 1 , 2 , 3  },   // facing forward
//        { 4 , 5 , 6 , 7  },   // walking left
//        { 8 , 9 , 10, 11 }   // walking right
//    };
    
    std::vector<std::vector<int>> enemy_animation_frames = {
        { 0 , 1 , 2 , 3  },   // blue
        { 4 , 5 , 6 , 7  },   // red
        { 8 , 9 , 10, 11 },   // yellow
        { 12, 13, 14, 15 },   // green right
        { 16, 17, 18, 19 },   // green left
        { 20, 21, 22, 23 },   // purple right
        { 24, 25, 26, 27 }    // purple left
    };
    
    glm::vec3 acceleration = glm::vec3(0.0f, -9.81f, 0.0f);
    
    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);
    
    m_game_state.player = new Entity(
        player_texture_id,         // texture id
        2.0f,                      // speed
        acceleration,              // acceleration
        8.0f,                      // jumping power
        player_animation,          // animation index sets
        0.0f,                      // animation time
        4,                         // animation frame amount
        0,                         // current animation index
        4,                         // animation column amount
        4,                         // animation row amount
        0.75f,                     // width
        1.0f,                      // height
        PLAYER
    );
        
    m_game_state.player->set_position(glm::vec3(20.0f, -10.0f, 0.0f));
    m_game_state.player->set_start_pos(m_game_state.player->get_position());
    
    for (int i = 0; i < 5; i++) {  // because there are five possible colors
        GLuint color_texture_id = Utility::load_texture(COLOR_FILEPATHS[i]);
        
        m_game_state.colors.push_back( new Entity( color_texture_id,          // texture id
                                                   2.0f,                      // speed
                                                   acceleration,              // acceleration
                                                   8.0f,                      // jumping power
                                                   player_animation,          // animation index sets
                                                   0.0f,                      // animation time
                                                   4,                         // animation frame amount
                                                   0,                         // current animation index
                                                   4,                         // animation column amount
                                                   4,                         // animation row amount
                                                   0.75f,                     // width
                                                   1.0f,                      // height
                                                   COLOR
                                               )
                                      );
        m_game_state.colors[i]->set_position(m_game_state.player->get_position());
        m_game_state.colors[i]->deactivate();
    }
    
    
    //Enemies' stuff
    
    // TODO: find better solution than dynamic cast (but not tonight)
    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);
    

    for (int i = 0; i < ENEMY_COUNT; i++) {
        
        m_game_state.enemies.push_back( new AI(enemy_texture_id,          // tex id
                                          1.0f + i * 0.1f,                      // speed
                                          acceleration,              // acceleration
                                          8.0f,                      // jumping power
                                          enemy_animation_frames,    // animation index sets
                                          0.0f,                      // animation time
                                          4,                         // animation frame amount
                                          0,                         // current animation index
                                          4,                         // animation column amount
                                          7,                         // animation row amount
                                          0.f,                      // width
                                          1.0f,                      // height
                                          ENEMY,                     // entity type
                                          PURPLE,                      // AI type
                                          IDLE)                     // AI state
                                       );
        m_game_state.collidables.push_back(m_game_state.enemies[i]);
        m_game_state.collidables[i]->set_movement(glm::vec3(0.0f));
        m_game_state.collidables[i]->set_acceleration(glm::vec3(0.0f, -3.73f, 0.0f));
    }
    
    for (int i = 0; i < ENEMY_COUNT; ++i) {
        m_game_state.enemies[i]->set_ai_type(static_cast<AIType>(i % 5));
    }
    
    // blue
    m_game_state.enemies[0]->set_position(glm::vec3(3.0f, -77.0f, 0.0f), 0, 3);
    m_game_state.enemies[5]->set_position(glm::vec3(21.0f, -14.0f, 0.0f), 0, 1);
    m_game_state.enemies[10]->set_position(glm::vec3(16.0f, -10.0f, 0.0f), 0, 10);
    m_game_state.enemies[15]->set_position(glm::vec3(30.0f, -52.0f, 0.0f), 0, 2);
    m_game_state.enemies[20]->set_position(glm::vec3(36.0f, -38.0f, 0.0f), 0, 3);
    
    // red
    m_game_state.enemies[1]->set_position(glm::vec3(34.0f, -43.0f, 0.0f), 5, 0);
    m_game_state.enemies[6]->set_position(glm::vec3(30.0f, -75.0f, 0.0f), 6, 0);
    m_game_state.enemies[11]->set_position(glm::vec3(10.0f, -55.0f, 0.0f), 2, 0);
    m_game_state.enemies[16]->set_position(glm::vec3(5.0f, -39.0f, 0.0f), 3, 0);
    m_game_state.enemies[21]->set_position(glm::vec3(47.0f, -47.0f, 0.0f), 2, 0);
    
    //yellow
    m_game_state.enemies[2]->set_position(glm::vec3(45.0f, -21.0f, 0.0f), 0, 0);
    m_game_state.enemies[7]->set_position(glm::vec3(36.0f, -75.0f, 0.0f), 0, 0);
    m_game_state.enemies[12]->set_position(glm::vec3(10.0f, -18.0f, 0.0f), 0, 0);
    m_game_state.enemies[17]->set_position(glm::vec3(30.0f, -31.0f, 0.0f), 0, 0);
    
    //green
    m_game_state.enemies[3]->set_position(glm::vec3(17.0f, -27.0f, 0.0f), 0, 0);
    m_game_state.enemies[8]->set_position(glm::vec3(5.0f, -68.0f, 0.0f), 0, 0);
    m_game_state.enemies[13]->set_position(glm::vec3(1.0f, -1.0f, 0.0f), 0, 0);
    m_game_state.enemies[18]->set_position(glm::vec3(20.0f, -42.0f, 0.0f), 0, 0);
    
    //purple
    m_game_state.enemies[4]->set_position(glm::vec3(15.0f, -10.0f, 0.0f), 0, 0);
    m_game_state.enemies[9]->set_position(glm::vec3(42.0f, -33.0f, 0.0f), 0, 0);
    m_game_state.enemies[14]->set_position(glm::vec3(29.0f, -16.0f, 0.0f), 0, 0);
    m_game_state.enemies[19]->set_position(glm::vec3(34.0f, -61.0f, 0.0f), 0, 0);
    
    GLuint target_texture_id = Utility::load_texture(TARGET_FILEPATH);
    
    std::vector<std::vector<int>> target_animation = {
        { 0 , 1 }
    };
    
    m_game_state.target = new Entity(
        target_texture_id,         // texture id
        2.0f,                      // speed
        acceleration,              // acceleration
        8.0f,                      // jumping power
        target_animation,          // animation index sets
        0.0f,                      // animation time
        2,                         // animation frame amount
        0,                         // current animation index
        2,                         // animation column amount
        1,                         // animation row amount
        1.0f,                      // width
        1.0f,                      // height
        PLATFORM
    );
    
    m_game_state.collidables.push_back(m_game_state.target);
    m_game_state.target->face_forward();
    m_game_state.target->set_position(glm::vec3(44.0f, -78.0f, 0.0f));
}

void LevelA::update(float delta_time)
{
    m_game_state.target->update(delta_time, m_game_state.player, m_game_state.collidables, 0, m_game_state.colors, m_game_state.map);
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.collidables, ENEMY_COUNT + 1, m_game_state.colors, m_game_state.map);
    
    bool enemies_vanquished = true;
    for (int i = 0; i < ENEMY_COUNT; i++) {
        AI* aiPtr = dynamic_cast<AI*>(m_game_state.collidables[i]);     // cast to AI, if possible
        if (aiPtr) {        // if AI
            aiPtr->update(delta_time, m_game_state.player, 1, m_game_state.map);
            if (m_game_state.collidables[i]->is_active()) {
                enemies_vanquished = false;
            }
        }
    }
    
    if (!m_game_state.player->is_active() || m_game_state.player->level_won()) {  // set active colors for next level
        for (int i = 0; i < 5; i++) {
            if (m_game_state.colors[i]->is_active()) {
                m_game_state.active_colors[i] = true;
            }
            else m_game_state.active_colors[i] = false;
        }
    }
    
    if (!m_game_state.player->is_active()) set_scene_id(4);  // switch to lose scene
    
    if (m_game_state.player->level_won()) {
        set_scene_id(5);    // winner!
        for (int i = 0; i < m_game_state.active_colors.size(); i ++) {
            if (m_game_state.active_colors[i]) set_scene_id(6);      // MURDERER
        }
    }
    
    for (int i = 0; i < m_game_state.colors.size(); i++) {
        m_game_state.colors[i]->update(delta_time, m_game_state.player, m_game_state.collidables, ENEMY_COUNT, m_game_state.colors, m_game_state.map);
    }
    
    m_game_state.map->update(m_game_state.player, delta_time);
    
    float player_x = m_game_state.player->get_position().x;
    float player_y = m_game_state.player->get_position().y;
    
    if (left_edge <= player_x && right_edge >= player_x) view_x = player_x;
    else if (left_edge > player_x) view_x = left_edge;
    else view_x = right_edge;
    
    if (top_edge >= player_y && bottom_edge <= player_y) view_y = player_y;
    else if (top_edge < player_y) view_y = top_edge;
    else view_y = bottom_edge;
    
    message_pos = glm::vec3(view_x + 7, view_y + 6.5, 0.0f);
    life_pos = glm::vec3(message_pos.x + 2, message_pos.y, 0.0f);

}

void LevelA::render(ShaderProgram *program)
{
    m_game_state.map->render(program);
    m_game_state.target->render(program);
    
    for (int i = 0; i < ENEMY_COUNT; i++) {
        m_game_state.collidables[i]->render(program);
    }
    
    m_game_state.player->render(program);
    
    for (int i = 0; i < m_game_state.colors.size(); i++) {
        m_game_state.colors[i]->render(program);
    }
    
    Utility::draw_text(program, font_texture_id, "LIVES: ", 0.3f, 0.01f, message_pos);
    Utility::draw_text(program, font_texture_id, std::to_string(m_game_state.player->get_lives() + 1),  0.3f, 0.01f, life_pos);
    
}
