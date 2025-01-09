#include <mellohi/core/assets/asset_manager.hpp>
#include <mellohi/core/assets/config_assets.hpp>
#include <mellohi/core/engine.hpp>
#include <mellohi/core/logger.hpp>

using namespace mellohi;

class SandboxGame : public Game
{
public:
    void init(Engine &engine) override
    {
        auto a = engine.get_engine_config_ptr();
        a->reload();
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
