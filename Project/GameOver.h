#include "Scene.h"

class GameOver : public Scene {
public:
    int ENEMY_COUNT = 5;
    GLuint L_font_texture_id;
    glm::vec3 message_pos;
    
    ~GameOver();
    
    void initialise(ShaderProgram *program) override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
