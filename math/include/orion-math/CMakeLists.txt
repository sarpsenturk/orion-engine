target_sources(
        orion-math INTERFACE
        FILE_SET public_headers
        FILES
        abs.h
        sqrt.h
        concepts.h
        constants.h
        angles.h
        trig.h
)

add_subdirectory(vector)
add_subdirectory(matrix)
