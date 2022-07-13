#include "orion/core/entry_point.h"

#ifdef ORION_MAIN
int main(int argc, const char* argv[])
{
    auto application = orion::create_application();
    application->on_create();
    application->run();
    application->on_shutdown();
    return 0;
}
#endif // ORION_MAIN
