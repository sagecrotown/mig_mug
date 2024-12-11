#include "Scene.h"

class LevelA : public Scene {
public:
    
    glm::vec3 message_pos;
    glm::vec3 life_pos;
    
    ~LevelA();
    
    void initialise(ShaderProgram *program) override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
