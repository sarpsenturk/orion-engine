#include "orion-core/module.h"

namespace orion
{
    Module::Module()
        : platform_module_(nullptr, nullptr)
    {
    }

    Module::Module(std::string filename)
        : platform_module_(platform::load_library(filename.c_str()), platform::free_library)
        , filename_(std::move(filename))
    {
    }
} // namespace orion
