/**
 * \file game.cpp
 * \date Jun 08, 2016
 */

#include "pang/game.hpp"

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

#include <cassert>
#include <cstdlib>

namespace pang {

    static std::string BIN_DIR;

    void setBinDir(const char* exeName)
    {
        char* out = realpath(exeName, NULL);
        if (out) {
            BIN_DIR.assign(out);
            free(out);
            auto pos = BIN_DIR.rfind('/');
            if (pos != std::string::npos) {
                BIN_DIR.resize(pos);
            } else {
                BIN_DIR.clear();
            }
        }
    }

    std::string getPath(const std::string& pth)
    {
        return BIN_DIR + "/" + pth;
    }

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
        auto& window = data.window;
        sf::Event ev;
        if (window.waitEvent(ev)) {
            switch (data.state) {
                case GameState::PLAYING:
                    window.clear(sf::Color(255, 0, 0));
                    window.display();
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

    static void showMainMenu(GameData& data)
    {
        assert(data.state == GameState::MENU);

        sf::Texture img;
        auto success = img.loadFromFile(getPath("resources/mainmenu.png"));
        assert(success);
        static_cast<void>(success);

        sf::Sprite sprite(img);

        auto& window = data.window;

        auto wsize = window.getSize();
        auto tsize = img.getSize();
        sprite.scale((float)wsize.x/tsize.x, (float)wsize.y/tsize.y);

        window.clear();
        window.draw(sprite);
        window.display();

        enum Buttons
        {
            PLAY = 0,
            QUIT = 1,
            kItems = 2
        };

        static const sf::Rect<int> kLocations[kItems] =
        {
            { 0, 145, 1023, 235 },
            { 0, 383, 1023, 177 }
        };

        auto handleClick =  [&data]
                            (int x, int y) -> bool
                            {
                                for (int i = 0; i < kItems; ++i) {
                                    if (kLocations[i].contains(x, y)) {
                                        if (i == QUIT) {
                                            data.state = GameState::EXITING;
                                        }
                                        return true;
                                    }
                                }
                                return false;
                            };

        sf::Event ev;
        while (true) {
            if (window.waitEvent(ev)) {
                switch (ev.type) {
                    case sf::Event::Closed:
                        data.state = GameState::EXITING;
                        goto show_menu_end;
                    case sf::Event::MouseButtonPressed:
                        if (ev.mouseButton.button == sf::Mouse::Left) {
                            if (handleClick(ev.mouseButton.x, ev.mouseButton.y)) {
                                goto show_menu_end;
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
        }

    show_menu_end:
        return;
    }

    static void showSplashScreen(GameData& data)
    {
        assert(data.state == GameState::SPLASH);

        sf::Texture img;
        auto success = img.loadFromFile(getPath("resources/splashscreen.png"));
        assert(success);
        static_cast<void>(success);

        sf::Sprite sprite(img);

        auto& window = data.window;
        window.clear();
        window.draw(sprite);
        window.display();

        // wait for keypress | mouse button | exit
        sf::Event ev;
        while (true) {
            if (window.waitEvent(ev)) {
                switch (ev.type) {
                    case sf::Event::Closed:// fall through
                        data.state = GameState::EXITING;
                    case sf::Event::KeyPressed:
                    case sf::Event::MouseButtonPressed:
                        goto show_splash_end;
                    default:
                        break;
                }
            }
        }

    show_splash_end:
        return;
    }

    void Game::start()
    {
        GameData& data = *m_impl;
        assert(data.state == GameState::UNINIT);

        // show splash screen
        data.state = GameState::SPLASH;
        showSplashScreen(data);
        if (data.state == GameState::EXITING) {
            return;
        }

        data.state = GameState::MENU;
        showMainMenu(data);
        if (data.state == GameState::EXITING) {
            return;
        }

        data.state = GameState::PLAYING;
        while (data.state != GameState::EXITING) {
            gameIteration(data);
        }
    }

}//namespace pang
