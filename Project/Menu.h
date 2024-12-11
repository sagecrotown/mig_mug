#include "Scene.h"
#include "Utility.h"

class Menu : public Scene {
public:
//    int ENEMY_COUNT = 1;
    GLuint Menu_font_texture_id;
    
    ~Menu();
    
    void initialise(ShaderProgram *program) override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
