#include "GameWon.h"
#include "Utility.h"

#define LEVEL_WIDTH 10
#define LEVEL_HEIGHT 10

constexpr char SPRITESHEET_FILEPATH[] = "assets/sage_miner.png",
           ENEMY_FILEPATH[]       = "assets/aiiiii.png",
            MAP_FILEPATH[] = "assets/ai_tile.png",
          FONT_FILEPATH[] = "assets/blue_font.png",
            CSV_FILEPATH[] = "assets/ai_game_over.csv";

constexpr char* COLOR_FILEPATHS[] = {
    "assets/blue_spot.png",
    "assets/red_spot.png",
    "assets/yellow_spot.png",
    "assets/green_spot.png",
    "assets/purple_spot.png",
};

std::vector<int> GAMEWON_DATA;

GameWon::~GameWon() {
    for (int i = 0; i < m_game_state.collidables.size(); i++) {
        delete m_game_state.collidables[i];
    }
    delete    m_game_state.player;
    delete    m_game_state.map;
//    Mix_FreeChunk(m_game_state.jump_sfx);
//    Mix_FreeMusic(m_game_state.bgm);
}

void GameWon::initialise(ShaderProgram *program) {

    m_game_state.next_scene_id = -1;
    
    Utility::readCSV(CSV_FILEPATH, GAMEWON_DATA, LEVEL_WIDTH * LEVEL_HEIGHT);
    
    GLuint map_texture_id = Utility::load_texture(MAP_FILEPATH);
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, GAMEWON_DATA, map_texture_id, 1.0f, 4, 1);
    
    W_font_texture_id = Utility::load_texture(FONT_FILEPATH);
    
    std::vector<std::vector<int>> player_animation = {
        { 0 },               // facing forward
        { 1 , 2 , 3 , 4 },   // walking left
        { 5 , 6 , 7 , 8 },   // walking right
        { 9 , 10 },           // digging down
        { 11, 12 },           // digging left
        { 13, 14 }            // digging right
    };
    
    std::vector<std::vector<int>> color_animation = {
        { 0 , 1 , 2 , 3  },   // facing forward
        { 4 , 5 , 6 , 7  },   // walking left
        { 8 , 9 , 10, 11 }   // walking right
    };
    
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
        
    m_game_state.player->set_position(glm::vec3(4.5f, -5.0f, 0.0f));
    
    for (int i = 0; i < 5; i++) {
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
        
        std::cout << m_game_state.active_colors[i] << std::endl;
        
        if (m_game_state.active_colors[i]) m_game_state.colors[i]->activate();
        else m_game_state.colors[i]->deactivate();
        
    }
    
    std::cout << "initialized colors" << std::endl;
    for (int i = 0; i < 5; i++) {
        std::cout << m_game_state.colors[i]->is_active() << std::endl;
    }
    
    
    
    //Enemies' stuff
    
    // TODO: find better solution than dynamic cast (but not tonight)
    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);

    
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
                                          BLUE,                      // AI type
                                          IDLE)                     // AI state
                                       );
        m_game_state.collidables[i]->deactivate();
    }
}

void GameWon::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.collidables, ENEMY_COUNT, m_game_state.collidables, m_game_state.map);
    
    for (int i = 0; i < m_game_state.colors.size(); i++) {
        m_game_state.colors[i]->update(delta_time, m_game_state.player, m_game_state.collidables, ENEMY_COUNT, m_game_state.colors, m_game_state.map);
    }
 
}

void GameWon::render(ShaderProgram *program)
{
    m_game_state.map->render(program);
    Utility::draw_text(program, W_font_texture_id, "WINNER WINNER", 0.5f, 0.01f, glm::vec3(1.25f, 2.0f , 0.0f));
    m_game_state.player->render(program);
    
    for (int i = 0; i < m_game_state.colors.size(); i++) {
        m_game_state.colors[i]->render(program);
    }
    
}
