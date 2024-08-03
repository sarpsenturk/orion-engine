target_sources(
        orion PRIVATE
        log.cpp
        application.cpp
)

# Configure file
configure_file(config.h.in ${PROJECT_BINARY_DIR}/conf/orion/config.h)
