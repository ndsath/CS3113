#include "Scene.h"

class MenuScreen : public Scene {
public:
    int ENEMY_COUNT = 1;
    
    
    ~MenuScreen();
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};

