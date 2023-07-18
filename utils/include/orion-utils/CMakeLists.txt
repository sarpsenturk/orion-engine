target_sources(
        orion-utils INTERFACE
        FILE_SET public_headers
        FILES
        allocators/allocator.h
        allocators/linear_allocator.h
        assertion.h
        bitflag.h
        functors.h
        hash.h
        static_vector.h
        type.h
        uninitialized.h
)
