target_sources(
        orion-utils INTERFACE
        FILE_SET public_headers
        FILES
        allocators/allocator.h
        allocators/linear_allocator.h
        array.h
        assertion.h
        bitflag.h
        bits.h
        callable.h
        compiler.h
        concepts.h
        expected.h
        finally.h
        hash.h
        minmax.h
        overload.h
        static_vector.h
        string.h
        type.h
        uninitialized.h
)
