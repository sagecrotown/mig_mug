#include "LevelC.h"
#include "Utility.h"

#define LEVEL_WIDTH 50
#define LEVEL_HEIGHT 30

constexpr char SPRITESHEET_FILEPATH[] = "assets/skel.png",
           ENEMY_FILEPATH[]       = "assets/aiiiii.png",
            MAP_FILEPATH[] = "assets/mars_tile.png",
          FONT_FILEPATH[] = "assets/blue_font.png",
            CSV_FILEPATH[] = "assets/platformer_3.csv",
            TARGET_FILEPATH[] = "assets/target.png";

constexpr char* COLOR_FILEPATHS[] = {
    "assets/blue_spot.png",
    "assets/red_spot.png",
    "assets/yellow_spot.png",
    "assets/green_spot.png",
    "assets/purple_spot.png",
};

LevelC::~LevelC() {
    for (int i = 0; i < m_game_state.collidables.size(); i++) {
        delete m_game_state.collidables[i];
    }
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelC::initialise(ShaderProgram *program) {

    m_game_state.next_scene_id = -1;
    m_game_state.LEVEL_DATA.resize(LEVEL_WIDTH * LEVEL_HEIGHT);
    
    Utility::readCSV(CSV_FILEPATH, m_game_state.LEVEL_DATA, LEVEL_WIDTH * LEVEL_HEIGHT);
    
    GLuint map_texture_id = Utility::load_texture(MAP_FILEPATH);
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, m_game_state.LEVEL_DATA, map_texture_id, 1.0f, 7, 3);
    
    C_font_texture_id = Utility::load_texture(FONT_FILEPATH);
    
    std::vector<std::vector<int>> player_animation = {
        { 0 , 1 , 2 , 3  },   // facing forward
        { 4 , 5 , 6 , 7  },   // walking left
        { 8 , 9 , 10, 11 }   // walking right
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
        3,                         // animation row amount
        0.75f,                     // width
        1.0f,                      // height
        PLAYER
    );
        
    m_game_state.player->set_position(glm::vec3(47.0f, -2.0f, 0.0f));
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
                                                   3,                         // animation row amount
                                                   0.75f,                     // width
                                                   1.0f,                      // height
                                                   COLOR
                                               )
                                      );
        m_game_state.colors[i]->set_position(m_game_state.player->get_position());
        if (m_game_state.active_colors[i]) m_game_state.colors[i]->activate();
        else m_game_state.colors[i]->deactivate();
       
    }
    
    
    //Enemies' stuff
    
    // TODO: find better solution than dynamic cast (but not tonight)
    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);
    
    std::vector<std::vector<int>> enemy_animation_frames = {
        { 0 , 1 , 2 , 3  },   // blue
        { 4 , 5 , 6 , 7  },   // red
        { 8 , 9 , 10, 11 },   // yellow
        { 12, 13, 14, 15 },   // green right
        { 16, 17, 18, 19 },   // green left
        { 20, 21, 22, 23 },   // purple right
        { 24, 25, 26, 27 }    // purple left
    };
    

    for (int i = 0; i < ENEMY_COUNT; i++) {
        
        m_game_state.collidables.push_back( new AI(enemy_texture_id,          // tex id
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
                                          YELLOW,                      // AI type
                                          IDLE)                     // AI state
                                       );
        m_game_state.collidables[i]->set_movement(glm::vec3(0.0f));
        m_game_state.collidables[i]->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    }
    
    m_game_state.collidables[0]->set_position(glm::vec3(20.0f, -2.0f, 0.0f));
    m_game_state.collidables[1]->set_position(glm::vec3(15.0f, -10.0f, 0.0f));
    m_game_state.collidables[2]->set_position(glm::vec3(40.0f, -5.0f, 0.0f));
    
    
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
    m_game_state.target->set_position(glm::vec3(42.0f, -18.0f, 0.0f));

    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_game_state.bgm = Mix_LoadMUS("assets/dooblydoo.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(0.0f);
    
    m_game_state.jump_sfx = Mix_LoadWAV("assets/bounce.wav");
}

void LevelC::update(float delta_time)
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

void LevelC::render(ShaderProgram *program)
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
    
    Utility::draw_text(program, C_font_texture_id, "LIVES: ", 0.3f, 0.01f, message_pos);
    Utility::draw_text(program, C_font_texture_id, std::to_string(m_game_state.player->get_lives() + 1),  0.3f, 0.01f, life_pos);
    
    
}
