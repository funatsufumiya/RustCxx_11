/*
 * Copyright (c) 2025 Dapeng Feng
 * All rights reserved.
 */

#pragma once
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace rust {

// Helper struct for creating overloaded visitors (C++17 compatible)
template <class... Ts>
struct overloads : Ts... {
  using Ts::operator()...;
};

// Helper for pattern matching - similar to Rust's match
template <typename Variant, class... Ts>
inline constexpr decltype(auto) match(Variant&& variant, Ts&&... ts) noexcept {
  return std::visit(overloads{std::forward<Ts>(ts)...},
                    std::forward<Variant>(variant));
}

// Enum-like wrapper around std::variant for better ergonomics
template <typename... Types>
class Enum {
 public:
  // Default constructor
  Enum() = default;

  // Constructor from any of the variant types
  template <typename T>
  // cppcheck-suppress  noExplicitConstructor
  Enum(T&& t) noexcept : value_(std::forward<T>(t)) {}  // NOLINT

  // Assignment
  template <typename T>
  Enum& operator=(T&& t) noexcept {
    value_ = std::forward<T>(t);
    return *this;
  }

  ~Enum() = default;

  // Get the index of the currently held type
  inline constexpr std::size_t index() const noexcept { return value_.index(); }

  // Check if the enum holds a specific type
  template <typename T>
  inline constexpr bool is() const noexcept {
    return std::holds_alternative<T>(value_);
  }

  // Get the value if it's of type T, throws if not
  template <typename T>
  inline T& get() & {
    if (!is<T>()) [[unlikely]] {
      throw std::bad_variant_access();
    }
    return std::get<T>(value_);
  }

  template <typename T>
  inline const T& get() const& {
    if (!is<T>()) [[unlikely]] {
      throw std::bad_variant_access();
    }
    return std::get<T>(value_);
  }

  template <typename T>
  inline T&& get() && {
    if (!is<T>()) [[unlikely]] {
      throw std::bad_variant_access();
    }
    return std::get<T>(std::move(value_));
  }

  // Get the value if it's of type T, returns nullptr if not
  template <typename T>
  inline T* get_if() noexcept {
    if (!is<T>()) [[unlikely]] {
      return nullptr;
    }
    return std::get_if<T>(&value_);
  }

  template <typename T>
  inline const T* get_if() const noexcept {
    if (!is<T>()) [[unlikely]] {
      return nullptr;
    }
    return std::get_if<T>(&value_);
  }

  // Match function for pattern matching
  template <typename... Ts>
  inline decltype(auto) match(Ts&&... ts) & noexcept {
    return rust::match(value_, std::forward<Ts>(ts)...);
  }

  template <typename... Ts>
  inline decltype(auto) match(Ts&&... ts) const& noexcept {
    return rust::match(value_, std::forward<Ts>(ts)...);
  }

  template <typename... Ts>
  inline decltype(auto) match(Ts&&... ts) && noexcept {
    return rust::match(std::move(value_), std::forward<Ts>(ts)...);
  }

  // Equality comparison
  inline bool operator==(const Enum& other) const noexcept {
    return value_ == other.value_;
  }

  inline bool operator!=(const Enum& other) const noexcept {
    return value_ != other.value_;
  }

 private:
  std::variant<Types...> value_;
};

// Rust-style Result type
template <typename T, typename E = std::string>
class Result {
 public:
  // Construct Ok result
  inline static constexpr Result Ok(T value) noexcept {
    return Result(std::move(value));
  }

  // Construct Err result
  inline static constexpr Result Err(E error) noexcept {
    return Result(std::move(error));
  }

  ~Result() = default;

  // Check if result is Ok
  inline constexpr bool is_ok() const noexcept {
    return std::holds_alternative<T>(value_);
  }

  // Check if result is Err
  inline constexpr bool is_err() const noexcept {
    return std::holds_alternative<E>(value_);
  }

  // Get the Ok value (throws if Err)
  inline T& unwrap() & {
    if (is_err()) [[unlikely]] {
      throw std::runtime_error("Called unwrap() on an Err Result");
    }
    return std::get<T>(value_);
  }

  inline const T& unwrap() const& {
    if (is_err()) [[unlikely]] {
      throw std::runtime_error("Called unwrap() on an Err Result");
    }
    return std::get<T>(value_);
  }

  inline T&& unwrap() && {
    if (is_err()) [[unlikely]] {
      throw std::runtime_error("Called unwrap() on an Err Result");
    }
    return std::get<T>(std::move(value_));
  }

  // Get the Ok value or a default
  template <typename U>
  inline T unwrap_or(U&& default_value) const& noexcept {
    return is_ok() ? std::get<T>(value_) : T(std::forward<U>(default_value));
  }

  template <typename U>
  inline T unwrap_or(U&& default_value) && noexcept {
    return is_ok() ? std::get<T>(std::move(value_))
                   : T(std::forward<U>(default_value));
  }

  // Get the error value (throws if Ok)
  inline E& unwrap_err() & {
    if (is_ok()) [[unlikely]] {
      throw std::runtime_error("Called unwrap_err() on an Ok Result");
    }
    return std::get<E>(value_);
  }

  inline const E& unwrap_err() const& {
    if (is_ok()) [[unlikely]] {
      throw std::runtime_error("Called unwrap_err() on an Ok Result");
    }
    return std::get<E>(value_);
  }

  // Map function - transform Ok value, leave Err unchanged
  template <typename F>
  inline decltype(auto) map(F&& f) noexcept {
    using result = Result<std::invoke_result_t<F, T>, E>;
    if (is_ok()) {
      return result::Ok(f(std::get<T>(value_)));
    } else {
      return result::Err(std::get<E>(value_));
    }
  }

  // Map error function - transform Err value, leave Ok unchanged
  template <typename F>
  inline decltype(auto) map_err(F&& f) noexcept {
    using result = Result<T, std::invoke_result_t<F, E>>;
    if (is_err()) {
      return result::Err(f(std::get<E>(value_)));
    } else {
      return result::Ok(std::get<T>(value_));
    }
  }

  // And then - chain Results
  template <typename F>
  inline decltype(auto) and_then(F&& f) noexcept {
    if (is_ok()) {
      return f(std::get<T>(value_));
    } else {
      using result = std::invoke_result_t<F, T>;
      return result::Err(std::get<E>(value_));
    }
  }

  // Pattern matching
  template <typename... Ts>
  inline decltype(auto) match(Ts&&... ts) & noexcept {
    return rust::match(value_, std::forward<Ts>(ts)...);
  }

  template <typename... Ts>
  inline decltype(auto) match(Ts&&... ts) const& noexcept {
    return rust::match(value_, std::forward<Ts>(ts)...);
  }

  template <typename... Ts>
  inline decltype(auto) match(Ts&&... ts) && noexcept {
    return rust::match(std::move(value_), std::forward<Ts>(ts)...);
  }

 private:
  template <typename U>
  explicit Result(U&& u) : value_(std::forward<U>(u)) {}

  std::variant<T, E> value_;
};

// Rust-style Option type
template <typename T>
class Option {
 public:
  // Construct Some option
  inline static constexpr Option Some(T value) noexcept {
    return Option(std::move(value));
  }

  // Construct None option
  inline static constexpr Option None() noexcept { return Option(); }

  // Default constructor creates None
  Option() : value_(std::nullopt) {}

  ~Option() = default;

  // Check if option is Some
  inline constexpr bool is_some() const noexcept { return value_.has_value(); }

  // Check if option is None
  inline constexpr bool is_none() const noexcept { return !value_.has_value(); }

  // Get the Some value (throws if None)
  inline T& unwrap() & {
    if (is_none()) {
      throw std::runtime_error("Called unwrap() on a None Option");
    }
    return *value_;
  }

  inline const T& unwrap() const& {
    if (is_none()) {
      throw std::runtime_error("Called unwrap() on a None Option");
    }
    return *value_;
  }

  inline T&& unwrap() && {
    if (is_none()) {
      throw std::runtime_error("Called unwrap() on a None Option");
    }
    return std::move(*value_);
  }

  // Get the Some value or a default
  template <typename U>
  inline T unwrap_or(U&& default_value) const& noexcept {
    return is_some() ? *value_ : T(std::forward<U>(default_value));
  }

  template <typename U>
  inline T unwrap_or(U&& default_value) && noexcept {
    return is_some() ? std::move(*value_) : T(std::forward<U>(default_value));
  }

  // Map function - transform Some value, leave None unchanged
  template <typename F>
  inline decltype(auto) map(F&& f) noexcept {
    using option = Option<std::invoke_result_t<F, T>>;
    if (is_some()) {
      return option::Some(f(*value_));
    } else {
      return option::None();
    }
  }

  // And then - chain Options
  template <typename F>
  inline decltype(auto) and_then(F&& f) noexcept {
    if (is_some()) {
      return f(*value_);
    } else {
      using option = std::invoke_result_t<F, T>;
      return option::None();
    }
  }

  // Pattern matching using variant-like interface
  template <typename SomeFunc, typename NoneFunc>
  inline decltype(auto) match(SomeFunc&& some_func,
                              NoneFunc&& none_func) noexcept {
    if (is_some()) {
      return some_func(*value_);
    } else {
      return none_func();
    }
  }

 private:
  explicit Option(T&& value) : value_(std::forward<T>(value)) {}

  std::optional<T> value_;
};

}  // namespace rust

// Convenience macros for defining enum variants
#define INTERNAL_ENUM_VARIANT_WITH_FIELDS(name, ...)                         \
  struct name {                                                              \
    __VA_ARGS__;                                                             \
    inline constexpr bool operator<=>(const name&) const noexcept = default; \
  }

// Helper macro for enum variants without fields.
// It takes only the name as an argument.
#define INTERNAL_ENUM_VARIANT_NO_FIELDS(name)                      \
  struct name {                                                    \
    inline constexpr bool operator==(const name&) const noexcept { \
      return true;                                                 \
    }                                                              \
    inline constexpr bool operator!=(const name&) const noexcept { \
      return false;                                                \
    }                                                              \
  }

// This macro acts as a dispatcher. It checks the number of arguments and
// selects the appropriate helper macro.
// If there's only one argument (the name), it selects
// INTERNAL_ENUM_VARIANT_NO_FIELDS. If there are more arguments (name and
// fields), it selects INTERNAL_ENUM_VARIANT_WITH_FIELDS.
#define GET_DISPATCH_MACRO(_1, _2, NAME, ...) NAME

#define ENUM_VARIANT(...)                                            \
  GET_DISPATCH_MACRO(__VA_ARGS__, INTERNAL_ENUM_VARIANT_WITH_FIELDS, \
                     INTERNAL_ENUM_VARIANT_NO_FIELDS)(__VA_ARGS__)
