#ifndef ORION_ENGINE_ENTRY_POINT_H
#define ORION_ENGINE_ENTRY_POINT_H

#include "application.h"
#include "engine.h"

namespace orion
{
    Application::Ptr create_application();
}

int main(int argc, const char* argv[])
{
    auto engine = orion::Engine(orion::create_application(), argc, argv);
    return engine.main();
}

#endif // ORION_ENGINE_ENTRY_POINT_H
