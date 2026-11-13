# AGENT Instructions

Orion is a custom engine for a KSP-like space game.

- Write simple, efficient, modern C++ code.
- Keep comments to explaining non-obvious constraints invariants, reasoning; avoid restating code. 
- Use fixed-width integer types from `<cstdint>`. Allow `std::size_t` for sizes and indexing, and built-in integer types where required by language or external API contracts.
- Use `std::span` for non-owning parameters representing contiguous sequences.
- Ensure changes are portable across the target platforms and compilers listed in README.md.
- After code or build-system changes, configure and build using the host platform’s Debug preset described in README.md. Report any validation you could not perform.