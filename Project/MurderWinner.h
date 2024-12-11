#include "Scene.h"

class MurderWinner : public Scene {
public:
    int ENEMY_COUNT = 3;
    GLuint M_font_texture_id;
    glm::vec3 message_pos;
    
    ~MurderWinner();
    
    void initialise(ShaderProgram *program) override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
