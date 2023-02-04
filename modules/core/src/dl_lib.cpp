#include "orion-core/dl_lib.h"

namespace orion
{
    Module::Module(std::string filename)
        : platform_module_(platform::load_library(filename.c_str()), {})
        , filename_(std::move(filename))
    {
    }
} // namespace orion
