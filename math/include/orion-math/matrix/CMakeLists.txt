target_sources(orion_math
        INTERFACE
        FILE_SET orion_math_headers
        TYPE HEADERS
        FILES
        matrix.h
        matrix2.h
        matrix3.h
        matrix4.h
        transformation.h)
