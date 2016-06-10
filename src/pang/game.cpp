/**
 * \file game.cpp
 * \date Jun 08, 2016
 */

#include "pang/game.hpp"

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

#include <cassert>
#include <cstdlib>

#include <iostream>

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

        const char * const PANG = "Pang!";
        auto allModes = sf::VideoMode::getFullscreenModes();

        // if (allModes.size() > 0) {
        //     m_impl->window.create(allModes[0], PANG, sf::Style::Fullscreen | sf::Style::Close);
        // } else {
            m_impl->window.create(sf::VideoMode::getDesktopMode(), PANG, sf::Style::Default);
        // }
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
          , m_window(window)
          , m_texture()
        {
        }

        bool loadFromFile(const std::string& s)
        {
            if (m_texture.loadFromFile(getPath(s))) {
                this->setTexture(m_texture);
                this->refreshScaling();
                return true;
            }
            return false;
        }

        void refreshScaling()
        {
            auto wsz = m_window.getSize();
            auto tsz = m_texture.getSize();
            this->setScale(static_cast<float>(wsz.x)/tsz.x, static_cast<float>(wsz.y)/tsz.y);
        }

        void setPosition(float, float)
        {
        }

        void setPosition(const sf::Vector2f&)
        {
        }
      private:
        sf::Window& m_window;
        sf::Texture m_texture;
    };

    using Image = GameObject<Scaling::ON>;
    using Item = GameObject<Scaling::OFF>;

    struct GameplayData
    {
        Item paddleA;
        Item paddleB;
        Item ball;

        sf::Vector2f posA;
        sf::Vector2f posB;
        sf::Vector2f posBall;

        sf::Clock clock;
    };

    template <class drawable>
    void setCenterPos(sf::Vector2u windowSize, drawable& d, sf::Vector2f pos)
    {
        auto tsize = d.getTexture()->getSize();
        sf::Vector2f tfsize(tsize);
        tfsize.x = tfsize.x * 0.5;
        tfsize.y = tfsize.y * 0.5;
        sf::Vector2f posw(pos.x * windowSize.x, pos.y*windowSize.y);
        d.setPosition(posw - tfsize);
    }

    static void updateGameplayItemState(sf::RenderWindow& window, GameplayData& game)
    {
        auto wsize = window.getSize();
        sf::Vector2f wfsize(wsize);

        setCenterPos(wsize, game.paddleA, game.posA);
        setCenterPos(wsize, game.paddleB, game.posB);
        setCenterPos(wsize, game.ball, game.posBall);
    }

    static void displayGameplayState(sf::RenderWindow& window, GameplayData& game)
    {
        window.clear();
        window.draw(game.paddleA);
        window.draw(game.paddleB);
        window.draw(game.ball);
        window.display();
    }

    static bool setupGameplayData(sf::RenderWindow& window, GameplayData& game)
    {
        bool success = game.paddleA.loadFromFile("resources/paddle.png") &&
                       game.paddleB.loadFromFile("resources/paddle.png") &&
                       game.ball.loadFromFile("resources/ball.png");
        if (!success) { return false; }

        game.posA = { 0.50f, 0.75f };
        game.posB = { 0.50f, 0.25f };
        game.posBall = { 0.50f, 0.50f };

        updateGameplayItemState(window, game);
        displayGameplayState(window, game);

        return true;
    }

    static const size_t FRAME_RATE = 40;
    static const int MILLIS_PER_FRAME = (1000/FRAME_RATE); 
    static const sf::Time TIME_PER_FRAME = sf::milliseconds(MILLIS_PER_FRAME);

    std::ostream& operator<<(std::ostream& s, const sf::Time& t)
    {
        s << t.asMilliseconds() << "ms";
        return s;
    }

    static void gameIteration(GameData& data, GameplayData& game)
    {
        assert(data.state == GameState::PLAYING);
        
        auto& window = data.window;
        
        game.clock.restart();

        sf::Event ev;
        if (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) {
                data.state = GameState::EXITING;
                return;
            }
        }

#ifndef NDEBUG
        auto timeTakenEventHandling = game.clock.getElapsedTime();
#endif

        switch (data.state) {
            case GameState::PLAYING:
            {

                game.posA += { 0.01f, 0.01f };
                game.posB -= { 0.01f, 0.0f };
                updateGameplayItemState(window, game);
#ifndef NDEBUG
                auto timeTakenUpdate = game.clock.getElapsedTime() - timeTakenEventHandling;
#endif
                displayGameplayState(window, game);

                auto elapsed = game.clock.getElapsedTime();
#ifndef NDEBUG
                auto timeTakenDisplay = elapsed - timeTakenUpdate;
                std::cerr << "[loop] Max " << TIME_PER_FRAME << ", Event " << timeTakenEventHandling << ", Update " << timeTakenUpdate << ", Display " << timeTakenDisplay << std::endl;
#endif
                if (elapsed < TIME_PER_FRAME) {
                    sf::sleep(TIME_PER_FRAME - elapsed);
                }
                break;
            }
            case GameState::EXITING:
                assert(false);
                break;
            default:
                break;
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

        enum Buttons
        {
            PLAY = 0,
            QUIT = 1,
            kItems = 2
        };

        static const sf::Rect<float> kLocations[kItems] = {
            { 0.0f, 145.0f/768.0f, 1.0f, 235.0f/768.0f },
            { 0.0f, 383.0f/768.0f, 1.0f, 177.0f/768.0f }
        };

        auto handleClick =  [&data, &window]
                            (int x, int y) -> bool
                            {
                                auto wsize = window.getSize();
                                auto fx = static_cast<float>(x)/wsize.x;
                                auto fy = static_cast<float>(y)/wsize.y;
                                for (int i = 0; i < kItems; ++i) {
                                    if (kLocations[i].contains(fx, fy)) {
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
            window.clear();
            window.draw(img);
            window.display();
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
                    case sf::Event::Resized:
                        img.refreshScaling();
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

        // wait for keypress | mouse button | exit
        sf::Event ev;
        while (true) {
            window.clear();
            window.draw(img);
            window.display();
            if (window.waitEvent(ev)) {
                switch (ev.type) {
                    case sf::Event::Closed:// fall through
                        data.state = GameState::EXITING;
                    case sf::Event::KeyPressed:
                    case sf::Event::MouseButtonPressed:
                        goto show_splash_end;
                    case sf::Event::Resized:
                        img.refreshScaling();
                        break;
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
