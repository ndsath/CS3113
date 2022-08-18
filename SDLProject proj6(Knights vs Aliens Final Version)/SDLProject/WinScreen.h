#include "Scene.h"

class WinScreen : public Scene {
public:
    int ENEMY_COUNT = 1;
    int POWER_COUNT = 1;
    
    
    ~WinScreen();
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};

