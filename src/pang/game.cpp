/**
 * \file game.cpp
 * \date Jun 08, 2016
 */

#include "pang/game.hpp"

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

#include <cassert>

namespace pang {

    enum class GameState
    {
        UNINIT,
        SPLASH,
        PAUSED,
        MENU,
        PLAYING,
        EXITING
    };

    struct GameData
    {
        GameState state;
        sf::RenderWindow window;
    };

    class Game::GameImpl : public GameData
    {
    };

    Game::Game() : m_impl(new Game::GameImpl)
    {
        m_impl->state = GameState::UNINIT;
        m_impl->window.create(sf::VideoMode(1024,768,32), "Pang!");
    }

    Game::~Game()
    {
        m_impl->window.close();
    }

    static void gameIteration(GameData& data)
    {
        sf::Event ev;
        if (data.window.waitEvent(ev)) {
            switch (data.state) {
                case GameState::PLAYING:
                    data.window.clear(sf::Color(255, 0, 0));
                    data.window.display();
                    if (ev.type == sf::Event::Closed) {
                        data.state = GameState::EXITING;
                    }
                    break;
                case GameState::EXITING:
                    assert(false);
                    break;
                default:
                    break;
            }
        }
    }

    void Game::start()
    {
        GameData& data = *m_impl;
        data.state = GameState::PLAYING;
        while (data.state != GameState::EXITING) {
            gameIteration(data);
        }
    }

}//namespace pang
