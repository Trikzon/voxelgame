#include "mellohi/game.h"

namespace mellohi
{
    Game::Game()
    {
        m_window = std::make_unique<Window>();
    }

    void Game::run()
    {
        on_run();

        while (!m_window->should_close())
        {
            if (auto frame = m_window->begin_frame())
            {
                auto [command_encoder, render_pass] = *frame;

                on_update(render_pass);

                m_window->end_frame(command_encoder, render_pass);
            }
        }
    }

    Window& Game::get_window() const
    {
        return *m_window;
    }
}