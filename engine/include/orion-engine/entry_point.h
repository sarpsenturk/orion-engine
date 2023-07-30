#pragma once

#include "orion-core/exception.h"

#include <cstdio>       // std::puts
#include <fmt/format.h> // fmt::format
#include <iterator>     // std::next
#include <span>         // std::span

#define ORION_MAIN(args)                                                   \
    int user_main(std::span<const char* const> args);                      \
    int main(int argc, const char* argv[])                                 \
    {                                                                      \
        try {                                                              \
            return user_main(std::span{argv, std::next(argv, argc)});      \
        } catch (const orion::OrionException& e) {                         \
            const auto what = fmt::format("[{}]: {}", e.type(), e.what()); \
            (void)std::fputs(what.c_str(), stderr);                        \
            return e.return_code();                                        \
        }                                                                  \
    }                                                                      \
    int user_main([[maybe_unused]] std::span<const char* const> args)
