#include "Scene.h"

class GameWon : public Scene {
public:
    int ENEMY_COUNT = 3;
    GLuint W_font_texture_id;
    glm::vec3 message_pos;
    
    ~GameWon();
    
    void initialise(ShaderProgram *program) override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
