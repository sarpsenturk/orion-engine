target_sources(
    orion-math INTERFACE
    FILE_SET public_headers
    FILES
    matrix.h
    matrix2.h
    matrix3.h
    matrix4.h
        projection.h
    transformation.h
)
