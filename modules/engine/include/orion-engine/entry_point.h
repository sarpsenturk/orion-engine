#pragma once

#include <iterator> // std::next
#include <span>     // std::span

#define ORION_MAIN(args)                                          \
    int user_main(std::span<const char* const> args);             \
    int main(int argc, const char* argv[])                        \
    {                                                             \
        return user_main(std::span{argv, std::next(argv, argc)}); \
    }                                                             \
    int user_main([[maybe_unused]] std::span<const char* const> args)
