target_sources(orion_math
        INTERFACE
        FILE_SET orion_math_headers
        TYPE HEADERS
        FILES
        abs.h
        sqrt.h
        func.h
        concepts.h
        constants.h
        angles.h
        trig.h
        )

add_subdirectory(vector)
add_subdirectory(matrix)
