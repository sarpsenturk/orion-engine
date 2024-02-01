# Style Guide For Orion

This file is a general code style guide for Orion.
It's a guide about the kind of C++ you should write for Orion.

Please note that Orion is not in a stable version yet.
Currently, some of these guidelines may be as volatile as the codebase itself.
If you happen to encounter any inconsistencies between this document
and the codebase itself or the config files for the tools such as
clang-tidy and clang-format trust the config file and codebase and
maybe create a pull request to correct it here :)

### clang-format

Orion uses clang-format to specify how code committed to the codebase
should be formatted. Make sure your code is formatted before you commit it.

Check out the [.clang-format](.clang-format) file for all the rules.

### clang-tidy

Orion uses clang-tidy for static analysis. Warnings generated
by clang-tidy should be taken seriously and fixed. If your IDE
supports it, integrate clang-tidy warnings with your IDE warnings.

Check out the [.clang-tidy](.clang-tidy) file for all the rules.
Identifier naming rules are also specified in the .clang-tidy file under `readability-identifier-naming`.
Check here for anything not specified
there.
If it's not specified here either, follow the convention used
in the function-class-file-module (in that order) you're working on.

Any exceptions to clang-format or identifier naming rules
(more context-sensitive rules) or anything not supported by those tools
are talked about here.

## Naming

Used casing styles are as clang-tidy calls
them ([see here](https://clang.llvm.org/extra/clang-tidy/checks/readability/identifier-naming.html)):

- snake_case: `int my_variable` & `int my_function()`
- CamelCase: `struct MyStruct` & `class MyClass`
- CAPITAL_CASE: `#define MY_MACRO`

Naming rules are checked by clang-tidy and show again here:

- Class types (`struct` & `class`) should use CamelCase
- Enum types and their values (`enum` & `enum class`) should use CamelCase
- Free functions should use snake_case
- Macros should use CAPITAL_CASE
- Member variables should use snake_case and post-fix `_` (`int counter_;`)
- Member functions should use snake_case
- Namespaces should use snake_case
- Parameters should use snake_case
- Template parameters should use CamelCase
- Variables should use snake_case

## Performance

Being a game engine, performance is essential for Orion.
However, premature optimization is very much discouraged.
Instead, write optimal code by default, in the most basic example,
prefer `++i` to `i++`, prefer `std::array` then `std::vector` then
`std::list` and etc., don't use dynamic allocations unless needed.

If, **through profiling and benchmarking**, a piece of code turns out
to be a performance issue, profile further. Make sure what you're intending
to optimize is the real issue. Once you're certain, optimize it, and
benchmark again. Ensure that your new optimized solution is better,
or worth the possible 'ugly' code.

See [Exceptions](#exceptions) for how to use exceptions properly.

RTTI is generally not used by Orion. It is not currently disabled
via compiler flags, however, meaning it's not prohibited. If a problem is
best solved with RTTI you are free to use it.

**These performance guidelines may change as the engine grows**. Orion
is currently a very small piece of software. It is not very performance
intensive in any way. It's very hard to say how the stuff we use today
will affect performance down the line.

## Assertions and Contracts

Orion provides 2 forms of runtime checks (assertions) for 2 different
types of problems:

- Assertions
- Contracts (kinda)

**Assertions and contracts SHOULD NOT contain any code which modifies
any state (local or global)!**

They are replaced with `(void)0` on higher optimization builds.
Meaning any state modifying code inside them will be gone and this
will lead to confusing errors that only appear in release builds.

## Error Handling

Error handling is a very complex topic so in Orion we try to make it
clear what error handling mechanism should be used where. There is simply
not any one size fits all solution.

Therefore, in Orion we use exceptions, error codes,
optionals (std::optional, nullptr) and a more modern solution
with [expected](https://en.cppreference.com/w/cpp/utility/expected).

All of these have places they're appropriate.

### Exceptions

Orion doesn't ban exceptions like many other game engines.
Exceptions are the preferred mechanism for when the caller might not be
the one to handle an error. Don't try to return a chain of error codes
up the call stack, use exceptions.

However, we're aware exception come with a cost.

- **Do not** throw and catch exceptions without any care for that cost.
- **Do not** use exceptions where error codes or optional values would
  work.
- **Try not to** use exceptions in hot or per frame code paths
- Use exceptions when needing to maintain invariants in constructors.

Code such as

```c++
auto obj = MyClass{arg};
if (obj.is_valid()) {...}
```

is simply not allowed.

If your class has an invariant to hold, throw an exception
when it can't. To avoid throwing when creating objects like such, prefer
factory methods which employ one of the other mentioned error handling
mechanisms.

I will reiterate the age-old saying that '*exceptions are for exceptional circumstances*'.
For Orion, that means situations that **even the caller** might not be able to
or want to solve.

Example:

```c++
class Window
{
public:
    // Should throw if window size nonsense for example
    explicit Window(const WindowDesc& desc);
};

enum class WindowCreateError {...};

// Validate arguments in factory and return expected, no exceptions
expected<Window, WindowCreateError> create_window(const WindowDesc& desc); 
```

### Assertions

Orion provides the `ORION_ASSERT(cond)` macro for assertions.
This should be used for debug only, runtime, sanity checks as it will
go away when running in release or distribution modes.

### Contracts

As of C++ 23 contracts are still not a part of the core language.
To mitigate this Orion provides macros `ORION_EXPECTS(cond)` and `ORION_ENSURES(cond)`
for pre-conditions and post-conditions respectively.

These should be used to validate user input to functions depending
on how wide or narrow the contract of the function is.

These macros go away **only** in distribution builds, therefore
you must be more careful with use of them in hot sections of code.

## Integer Types

Use of builtin integer types (int, unsigned, long, short, etc.) are
almost always avoided. Use sized type aliases provided by the standard
library header `<cstdint>`. Single exception may be the use of keyword
`int` as it's almost always known to be 32 bits. Even then consider using
`std::int32_t`.

```c++
unsigned int i = 42;    // BAD!
uint32_t j = 42;        // Good, using 'deprecated' C aliases
std::uint32_t j = 12;   // Good

long long j = 42;       // BAD!
std::int64_t j = 42;    // Good
```

Use of C-like aliases are allowed, most of the codebase uses the C++
version but there's no issue using `uint32_t` over `std::uint32_t`

## Type Aliases

Orion uses modern C++'s `using` for type aliases.
`typedef` is not allowed and will give you a clang-tidy warning (see [here](https://clang.llvm.org/extra/clang-tidy/checks/modernize/use-using.html))

### Aliases to Primitive Types

Aliases to builtin/primitive types (int, unsigned, float)
or their standard library aliases (std::uint32_t, std::int8_t)
use snake_case and are post-fixed with `_t`

Example:

```c++
using i32_t = std::int32_t;
using my_key_t = std::uint64_t;
```

### Aliases to User Defined Types

Aliases to user defined types are generally in PascalCase and aren't
pre-fixed or post-fixed by anything.

Example:

```c++
using StorageType = std::vector<int>;

class Vector4{...};
using Color = Vector4;
```

This is generally so that an
alias to a user-defined/complex type doesn't hide that the aliased
type is potentially non-trivial, both from a language perspective
and from users perspective.

If the user defined type you're aliasing is a trivial type, using the
same style as primitive types may be ok. An example to this, are
aliases to the `Handle` type provided in orion-core.

### Class Scope Aliases

Everything since has referred to aliases in global or function scope.
Class scope aliases almost always follow the standard library
convention of snake_case regardless of if the type is primitive, trivial,
user defined, etc.

Example:

```c++
class MyClass
{
public:
    using value_type = std::int32_t;
    using container_type = std::vector<value_type>;
};
```

Exceptions to this would be rare and would need a strong argument.

## Virtual Functions

Orion doesn't discourage the use of virtual functions or dynamic dispatch
in any way.

However, if the problem doesn't need runtime dispatch,
consider a non-virtual solution again. Compile time polymorphism
can provide much better performance if used properly.
If you believe the virtual solution is cleaner, simpler, go for it.

### Non-virtual Interfaces

Interfaces in Orion follow the *non-virtual interface (NVI)* pattern.
You can read more about what that is [here](https://en.wikipedia.org/wiki/Non-virtual_interface_pattern).

NVI allows for simpler pre-conditions and post-conditions,

### Virtual Function Style

Overridden functions must be marked down with `override` and optionally
`final`. `virtual` keyword is only used in base classes only.

```c++
struct MyInterface
{
    virtual void f() = 0;
};

struct MyImpl : MyInterface
{
    virtual void f();           // BAD: Don't use virtual if it's implemented
    virtual void f() override;  // BAD: Override is more than enough
    void f() override;          // Good
};
```
