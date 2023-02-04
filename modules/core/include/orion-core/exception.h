#pragma once

#include <stdexcept> // std::runtime_error

namespace orion
{
    class OrionException : public std::runtime_error
    {
    public:
        OrionException()
            : std::runtime_error("Orion runtime error")
        {
        }

        explicit OrionException(const std::string& message)
            : std::runtime_error(message)
        {
        }

        [[nodiscard]] virtual const char* type() const noexcept = 0;
        [[nodiscard]] virtual int return_code() const noexcept { return -1; }
    };
} // namespace orion
