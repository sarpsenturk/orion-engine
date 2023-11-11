target_sources(
        orion-utils INTERFACE
        FILE_SET public_headers
        FILES
        allocators/allocator.h
        allocators/linear_allocator.h
        assertion.h
        bitflag.h
        compiler.h
        expected.h
        functors.h
        hash.h
        overload.h
        static_vector.h
        string.h
        type.h
        uninitialized.h
)
