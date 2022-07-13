#ifndef ORION_ENGINE_ENTRY_POINT_H
#define ORION_ENGINE_ENTRY_POINT_H

#include "application.h"

#include <memory>

namespace orion
{
    std::unique_ptr<Application> create_application();
}

int main(int argc, const char* argv[])
{
    auto application = orion::create_application();
    application->on_create();
    application->run();
    application->on_shutdown();
    return 0;
}

#endif // ORION_ENGINE_ENTRY_POINT_H
