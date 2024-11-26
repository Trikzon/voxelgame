#include <mellohi/core/engine.hpp>
#include <mellohi/core/logger.hpp>

using namespace mellohi;

class SandboxGame : public Game
{
public:
    void init(Engine &engine) override
    {
        for (const auto &extension : engine.get_platform_ptr()->get_required_vulkan_instance_extensions())
        {
            MH_WARN("{}", extension);
        }
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
