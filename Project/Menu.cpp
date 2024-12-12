#include "Menu.h"
#include "Utility.h"

#define LEVEL_WIDTH 50
#define LEVEL_HEIGHT 20

constexpr char SPRITESHEET_FILEPATH[] = "assets/skel.png",
           ENEMY_FILEPATH[]       = "assets/aiiiii.png",
            MAP_FILEPATH[] = "assets/mars_tile.png",
          FONT_FILEPATH[] = "assets/blue_font.png",
            CSV_FILEPATH[] = "assets/platformer_1.csv";

constexpr char* COLOR_FILEPATHS[] = {
    "assets/blue_spot.png",
    "assets/red_spot.png",
    "assets/yellow_spot.png",
    "assets/green_spot.png",
    "assets/purple_spot.png",
};

Menu::~Menu() {
    for (int i = 0; i < m_game_state.collidables.size(); i++) {
        delete m_game_state.collidables[i];
    }
    delete    m_game_state.player;
    delete    m_game_state.map;
//    Mix_FreeChunk(m_game_state.jump_sfx);
//    Mix_FreeMusic(m_game_state.bgm);
}

void Menu::initialise(ShaderProgram *program) {

    m_game_state.next_scene_id = -1;
    m_game_state.LEVEL_DATA.resize(LEVEL_WIDTH * LEVEL_HEIGHT);
    
    Utility::readCSV(CSV_FILEPATH, m_game_state.LEVEL_DATA, LEVEL_WIDTH * LEVEL_HEIGHT);
    
    GLuint map_texture_id = Utility::load_texture(MAP_FILEPATH);
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, m_game_state.LEVEL_DATA, map_texture_id, 1.0f, 4, 1);
    
    Menu_font_texture_id = Utility::load_texture(FONT_FILEPATH);
    
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
        
    m_game_state.player->set_position(glm::vec3(10.0f, -5.0f, 0.0f));
    m_game_state.player->deactivate();
    
    for (int i = 0; i < ENEMY_COUNT; i++) {
        GLuint color_texture_id = Utility::load_texture(COLOR_FILEPATHS[i]);
//        GLuint color_texture_id = Utility::load_texture(BLUE_FILEPATH);
        
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
        m_game_state.colors[i]->deactivate();
    }
    
    for (int i = 0; i < 5; i++) {
        m_game_state.active_colors.push_back(false);
    }
    
    m_game_state.player->set_lives(2);   // this is three lives
    
//    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);
    m_game_state.collidables.push_back(new AI());
}

void Menu::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.collidables, ENEMY_COUNT, m_game_state.colors, m_game_state.map);
}

void Menu::render(ShaderProgram *program)
{
//    m_game_state.map->render(program);
    Utility::draw_text(program, Menu_font_texture_id, "MIG MUG", 1.5f, 0.01f, glm::vec3(5.50f, -4.0f , 0.0f));
//    Utility::draw_text(program, Menu_font_texture_id, "MUSIC BY EL WHITTLE", 0.4f, 0.01f, glm::vec3(6.5f, -6.0f , 0.0f));
//    Utility::draw_text(program, Menu_font_texture_id, "EVERYTHING ELSE BY SAGE CRONEN-TOWNSEND", 0.4f, 0.01f, glm::vec3(2.0f, -7.0f , 0.0f));
    Utility::draw_text(program, Menu_font_texture_id, "PRESS ENTER TO START", 0.3f, 0.01f, glm::vec3(1.0f, -13.0f , 0.0f));
}
