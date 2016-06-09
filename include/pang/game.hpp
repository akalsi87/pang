/**
 * \file game.hpp
 * \date Jun 08, 2016
 */

#ifndef PANG_GAME_HPP
#define PANG_GAME_HPP

#include "pang/types.hpp"

#include <memory>

namespace pang {

    void setBinDir(const char* exeName);

    class Game : NonCopyable
    {
      public:
        Game();

        ~Game();

        void start();

      private:
        class GameImpl;

        std::unique_ptr<GameImpl> m_impl;
    };

}//namespace pang

#endif/*PANG_GAME_HPP*/
