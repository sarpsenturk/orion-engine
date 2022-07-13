#ifndef ORION_ENGINE_ENTRY_POINT_H
#define ORION_ENGINE_ENTRY_POINT_H

#include "application.h"

#include <memory>

namespace orion
{
    std::unique_ptr<Application> create_application();
}

int main(int argc, const char* argv[]);

#endif // ORION_ENGINE_ENTRY_POINT_H
