target_sources(
        orion-utils INTERFACE
        FILE_SET public_headers
        FILES
        assertion.h
        bitflag.h
        functors.h
        hash.h
        static_vector.h
        type.h
        uninitialized.h
)
