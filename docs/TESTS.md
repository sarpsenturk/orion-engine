# Tests

- GoogleTest is used for testing.
- Define all tests in an anonymous namespaces inside the `orion` namespace.
- Declare all engine tests in `engine/tests/CMakeLists.txt`
- All test source files should follow `[name].test.cpp`
- Use our CMake helper function `orion_add_test` to add tests
