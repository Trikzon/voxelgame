#include <mellohi/core/engine.hpp>
#include <mellohi/core/logger.hpp>

using namespace mellohi;

class SandboxGame : public Game
{
public:
    void init(Engine &engine) override
    {
        
    }
    
    void process(Engine &engine) override
    {
        
    }
};

int main()
{
    SandboxGame game;
    Engine engine;
    engine.run(game);
}
