#include "Scene.h"

class LossScreen : public Scene {
public:
    int ENEMY_COUNT = 1;
    int POWER_COUNT = 1;
    
    
    ~LossScreen();
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};

