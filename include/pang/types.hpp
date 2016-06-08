/**
 * \file types.hpp
 * \date Jun 08, 2016
 */

#ifndef PANG_TYPES_HPP
#define PANG_TYPES_HPP

#include <cstdint>
#include <cstddef>

namespace pang {

    class NonCopyable
    {
      public:
        NonCopyable() = default;
        ~NonCopyable() = default;
        NonCopyable(const NonCopyable&) = delete;
        NonCopyable& operator=(const NonCopyable&) = delete;
    };

}//namespace pang

#endif/*PANG_TYPES_HPP*/
