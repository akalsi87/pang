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

    enum class Scaling
    {
        ON,
        OFF
    };

    template <Scaling sc = Scaling::OFF>
    class GameObject;

    template <>
    class GameObject<Scaling::OFF> : public sf::Sprite
    {
      public:
        GameObject()
          : Sprite()
          , m_texture()
        {
        }

        GameObject(sf::Window&)
          : Sprite()
          , m_texture()
        {
        }

        bool loadFromFile(const std::string& s)
        {
            if (m_texture.loadFromFile(getPath(s))) {
                this->setTexture(m_texture);
                return true;
            }
            return false;
        }
      private:
        sf::Texture m_texture;
    };

    template <>
    class GameObject<Scaling::ON> : public sf::Sprite
    {
      public:
        GameObject(sf::Window& window)
          : Sprite()
          , m_windowSize(window.getSize())
          , m_texture()
        {
        }

        bool loadFromFile(const std::string& s)
        {
            if (m_texture.loadFromFile(getPath(s))) {
                auto wsize = m_windowSize;
                auto tsize = m_texture.getSize();
                this->setTexture(m_texture);
                this->scale((float)wsize.x/tsize.x, (float)wsize.y/tsize.y);
                return true;
            }
            return false;
        }

        void setPosition(float, float)
        {
        }

        void setPosition(const sf::Vector2f&)
        {
        }
      private:
        sf::Vector2u m_windowSize;
        sf::Texture m_texture;
    };

    using Image = GameObject<Scaling::ON>;
    using Item = GameObject<Scaling::OFF>;

    struct GameplayData
    {
        Item paddleA;
        Item paddleB;
        Item ball;
    };

    template <class drawable>
    void setCenterTo(drawable& d, sf::Vector2f pos)
    {
        auto tsize = d.getTexture()->getSize();
        sf::Vector2f tfsize(tsize);
        tfsize.x = tfsize.x / 2;
        tfsize.y = tfsize.y / 2;
        d.setPosition(pos - tfsize);
    }

    static bool setupGameplayData(sf::RenderWindow& window, GameplayData& game)
    {
        bool success = game.paddleA.loadFromFile("resources/paddle.png") &&
                       game.paddleB.loadFromFile("resources/paddle.png") &&
                       game.ball.loadFromFile("resources/ball.png");
        if (!success) { return false; }

        auto wsize = window.getSize();
        sf::Vector2f wfsize(wsize);

        setCenterTo(game.paddleA, { wfsize.x * 0.50f, wfsize.y * 0.75f });
        setCenterTo(game.ball,    { wfsize.x * 0.50f, wfsize.y * 0.50f });
        setCenterTo(game.paddleB, { wfsize.x * 0.50f, wfsize.y * 0.25f });

        return true;
    }

    static void displayGameplayState(sf::RenderWindow& window, GameplayData& game)
    {
        window.clear();
        window.draw(game.paddleA);
        window.draw(game.paddleB);
        window.draw(game.ball);
        window.display();
    }

    static void gameIteration(GameData& data, GameplayData& game)
    {
        assert(data.state == GameState::PLAYING);
        auto& window = data.window;
        sf::Event ev;
        if (window.waitEvent(ev)) {
            switch (data.state) {
                case GameState::PLAYING:
                    displayGameplayState(window, game);
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

        auto& window = data.window;

        Image img(window);
        auto success = img.loadFromFile("resources/mainmenu.png");
        assert(success);
        static_cast<void>(success);

        window.clear();
        window.draw(img);
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

        auto& window = data.window;

        Image img(window);
        auto success = img.loadFromFile("resources/splashscreen.png");
        assert(success);
        static_cast<void>(success);

        window.clear();
        window.draw(img);
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

        // show menu
        data.state = GameState::MENU;
        showMainMenu(data);
        if (data.state == GameState::EXITING) {
            return;
        }

        // start game
        GameplayData game;
        bool success = setupGameplayData(data.window, game);
        assert(success);
        static_cast<void>(success);

        data.state = GameState::PLAYING;
        while (data.state != GameState::EXITING) {
            gameIteration(data, game);
        }
    }

}//namespace pang
