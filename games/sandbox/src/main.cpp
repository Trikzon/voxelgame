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
        
    }
    
    void process(Engine &engine) override
    {
        // TODO: Move to a command.
        if (engine.get_platform_ptr()->reload_pressed())
        {
            MH_TRACE("Reload all assets.");
            engine.get_asset_manager_ptr()->reload_all();
        }
    }
};

int main()
{
    SandboxGame game;
    Engine engine;
    engine.run(game);
}
