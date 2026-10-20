# AI Agent Guidelines

- Write simple, efficient, minimal C/C++ code.
- Write self documenting code, avoid unnecessary comments.
- Avoid overly defensive code unless validation is part of the requirements. (e.g., an allocation holding a reference to the allocator to ensure `deallocate` was called with the correct owner)
- Do not try to handle unrecoverable errors such as memory allocation failures.
- Avoid copying large data structures. Prefer references or pointers, use spans for array data in structures and function parameters.
- Always use standard C++ fixed width integer types.
- Use C++ 20 designated initializers with named fields for structures.
- All code is subject to change, no public API is finalized yet.
- Do not proactively write tests, only do so when the user asks.

## Further Context

Read the relevant document(s) before working in its area:

- [Building](docs/BUILD.md): Configuration, build steps, requirements.
- [Testing](docs/TESTS.md): Writing, configurnig, executing tests.
- [Renderer](docs/RENDERER.md): GPU backend, render graph, frame flow, renderer resources.