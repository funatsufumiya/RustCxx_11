# RustCxx - Rust-style Enums for C++

RustCxx is a modern C++20 header-only library that provides Rust-style enums, `Result`, and `Option` types for C++. It's designed to bring the expressiveness and safety of Rust's type system to C++.

## Features

- 🚀 **Header-only**: Just include and use
- 🦀 **Rust-inspired**: Familiar API for Rust developers
- 🔒 **Type-safe**: Compile-time type checking
- 🎯 **Pattern Matching**: Expressive pattern matching with lambdas
- ⚡ **Modern C++**: Requires C++20, uses `std::variant` under the hood
- 🧪 **Well-tested**: Comprehensive test suite

## Quick Start

### Basic Enum Usage

```cpp
#include "rustcxx/rustcxx.hpp"
using namespace rustcxx;

// Define enum variants
ENUM_VARIANT(Red);
ENUM_VARIANT(Green);
ENUM_VARIANT(Blue, int intensity);

using Color = Enum<Red, Green, Blue>;

// Usage
Color color = Blue{128};

// Pattern matching
auto description = color.match(
    [](const Red&) { return "It's red!"; },
    [](const Green&) { return "It's green!"; },
    [](const Blue& b) {
        return "It's blue with intensity " + std::to_string(b.intensity);
    }
);
```

### Result Type

```cpp
auto divide = [](int a, int b) -> Result<double, std::string> {
    if (b == 0) {
        return Result<double, std::string>::Err("Division by zero!");
    }
    return Result<double, std::string>::Ok(static_cast<double>(a) / b);
};

auto result = divide(10, 2);

// Pattern matching
result.match(
    [](double value) { std::cout << "Result: " << value << std::endl; },
    [](const std::string& error) { std::cout << "Error: " << error << std::endl; }
);

// Safe access
double value = result.unwrap_or(-1.0);

// Chaining
auto chained = divide(20, 4)
    .map([](double x) { return x * 2; })
    .and_then([&](double x) { return divide(static_cast<int>(x), 2); });
```

### Option Type

```cpp
auto find_user = [](int id) -> Option<std::string> {
    if (id == 1) return Option<std::string>::Some("Alice");
    if (id == 2) return Option<std::string>::Some("Bob");
    return Option<std::string>::None();
};

auto user = find_user(1);

// Pattern matching
user.match(
    [](const std::string& name) { std::cout << "Found: " << name << std::endl; },
    []() { std::cout << "User not found" << std::endl; }
);

// Chaining operations
auto greeting = find_user(1)
    .map([](const std::string& name) { return "Hello, " + name + "!"; })
    .unwrap_or("Hello, stranger!");
```

## Building

### Requirements

- C++20 compatible compiler
- CMake 3.14 or later

### Build Examples

```bash
mkdir build && cd build
cmake ..
make
./basic_usage
```

### Run Tests

```bash
cmake -DRUSTCXX_BUILD_TESTS=ON ..
make
ctest
```

## API Reference

### Enum<Types...>

Core enum type that wraps `std::variant` with ergonomic APIs.

#### Methods

- `bool is<T>() const` - Check if enum holds type T
- `T& get<T>()` - Get value of type T (throws if wrong type)
- `T* get_if<T>()` - Get pointer to value if type T, nullptr otherwise
- `auto match(lambdas...)` - Pattern match over all variants
- `std::size_t index() const` - Get index of current variant

### Result<T, E>

Type-safe error handling similar to Rust's `Result<T, E>`.

#### Static Methods

- `Result::Ok(value)` - Create successful result
- `Result::Err(error)` - Create error result

#### Methods

- `bool is_ok() const` - Check if result is Ok
- `bool is_err() const` - Check if result is Err
- `T unwrap()` - Get Ok value (throws if Err)
- `T unwrap_or(default)` - Get Ok value or default
- `E unwrap_err()` - Get Err value (throws if Ok)
- `auto map(func)` - Transform Ok value
- `auto map_err(func)` - Transform Err value
- `auto and_then(func)` - Chain Results
- `auto match(ok_func, err_func)` - Pattern match

### Option<T>

Nullable type similar to Rust's `Option<T>`.

#### Static Methods

- `Option::Some(value)` - Create Some option
- `Option::None()` - Create None option

#### Methods

- `bool is_some() const` - Check if option has value
- `bool is_none() const` - Check if option is empty
- `T unwrap()` - Get value (throws if None)
- `T unwrap_or(default)` - Get value or default
- `auto map(func)` - Transform Some value
- `auto and_then(func)` - Chain Options
- `auto match(some_func, none_func)` - Pattern match

## Macros

- `ENUM_VARIANT(name)` - Define empty variant

## Integration

### CMake

If installed:

```cmake
find_package(rustcxx REQUIRED)
target_link_libraries(your_target rustcxx::rustcxx)
```

Or as subdirectory:

```cmake
add_subdirectory(path/to/rustcxx)
target_link_libraries(your_target rustcxx)
```

### Direct Include

Simply copy `include/rustcxx/rustcxx.hpp` to your project and include it.

## Examples

See the `examples/` directory for more comprehensive examples:

- `basic_usage.cpp` - Demonstrates all core features
- More examples coming soon!

## Comparison with Rust

| Rust | RustCxx |
|------|-------|
| `enum Color { Red, Green, Blue(u8) }` | `ENUM_VARIANT(Red); ENUM_VARIANT(Red); ENUM_VARIANT(Blue, uint8_t intensity); using Color = Enum<Red, Green, Blue>;` |
| `Result<T, E>` | `Result<T, E>` |
| `Option<T>` | `Option<T>` |
| `match expr { ... }` | `expr.match(...)` |
| `result?` | `result.and_then(...)` |
| `option.map(f)` | `option.map(f)` |

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

Licensed under the Apache License, Version 2.0. See [LICENSE](LICENSE) for details.

## Acknowledgments

Inspired by Rust's excellent type system and the many discussions about bringing similar functionality to C++.
