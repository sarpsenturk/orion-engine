#include "orion-core/dyn_lib.h"

#include "orion-utils/assertion.h"

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

    void* Module::load_symbol(const char* symbol_name) const
    {
        ORION_EXPECTS(platform_module_ != nullptr);
        return platform::load_library_address(platform_module_.get(), symbol_name);
    }
} // namespace orion
