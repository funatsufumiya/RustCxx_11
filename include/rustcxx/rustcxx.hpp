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

#if __cplusplus < 202002L
template <class... Ts>
overloads(Ts...) -> overloads<Ts...>;
#endif

// Helper for pattern matching - similar to Rust's match
template <typename Variant, class... Ts>
constexpr auto match(Variant&& variant, Ts&&... ts) -> decltype(auto) {
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
  Enum(T&& t) : value_(std::forward<T>(t)) {}  // NOLINT

  // Assignment
  template <typename T>
  Enum& operator=(T&& t) {
    value_ = std::forward<T>(t);
    return *this;
  }

  ~Enum() = default;

  // Get the index of the currently held type
  constexpr std::size_t index() const noexcept { return value_.index(); }

  // Check if the enum holds a specific type
  template <typename T>
  constexpr bool is() const noexcept {
    return std::holds_alternative<T>(value_);
  }

  // Get the value if it's of type T, throws if not
  template <typename T>
  T& get() & {
    return std::get<T>(value_);
  }

  template <typename T>
  const T& get() const& {
    return std::get<T>(value_);
  }

  template <typename T>
  T&& get() && {
    return std::get<T>(std::move(value_));
  }

  // Get the value if it's of type T, returns nullptr if not
  template <typename T>
  T* get_if() noexcept {
    return std::get_if<T>(&value_);
  }

  template <typename T>
  const T* get_if() const noexcept {
    return std::get_if<T>(&value_);
  }

  // Match function for pattern matching
  template <typename... Ts>
  auto match(Ts&&... ts) & -> decltype(auto) {
    return rust::match(value_, std::forward<Ts>(ts)...);
  }

  template <typename... Ts>
  auto match(Ts&&... ts) const& -> decltype(auto) {
    return rust::match(value_, std::forward<Ts>(ts)...);
  }

  template <typename... Ts>
  auto match(Ts&&... ts) && -> decltype(auto) {
    return rust::match(std::move(value_), std::forward<Ts>(ts)...);
  }

  // Equality comparison
  bool operator==(const Enum& other) const { return value_ == other.value_; }

  bool operator!=(const Enum& other) const { return value_ != other.value_; }

 private:
  std::variant<Types...> value_;
};

// Rust-style Result type
template <typename T, typename E = std::string>
class Result {
 public:
  // Construct Ok result
  static Result Ok(T value) { return Result(value); }

  // Construct Err result
  static Result Err(E error) { return Result(error); }

  ~Result() = default;

  // Check if result is Ok
  constexpr bool is_ok() const noexcept {
    return std::holds_alternative<T>(value_);
  }

  // Check if result is Err
  constexpr bool is_err() const noexcept {
    return std::holds_alternative<E>(value_);
  }

  // Get the Ok value (throws if Err)
  T& unwrap() & {
    if (is_err()) {
      throw std::runtime_error("Called unwrap() on an Err Result");
    }
    return std::get<T>(value_);
  }

  const T& unwrap() const& {
    if (is_err()) {
      throw std::runtime_error("Called unwrap() on an Err Result");
    }
    return std::get<T>(value_);
  }

  T&& unwrap() && {
    if (is_err()) {
      throw std::runtime_error("Called unwrap() on an Err Result");
    }
    return std::get<T>(std::move(value_));
  }

  // Get the Ok value or a default
  template <typename U>
  T unwrap_or(U&& default_value) const& {
    return is_ok() ? std::get<T>(value_) : T(std::forward<U>(default_value));
  }

  template <typename U>
  T unwrap_or(U&& default_value) && {
    return is_ok() ? std::get<T>(std::move(value_))
                   : T(std::forward<U>(default_value));
  }

  // Get the error value (throws if Ok)
  E& unwrap_err() & {
    if (is_ok()) {
      throw std::runtime_error("Called unwrap_err() on an Ok Result");
    }
    return std::get<E>(value_);
  }

  const E& unwrap_err() const& {
    if (is_ok()) {
      throw std::runtime_error("Called unwrap_err() on an Ok Result");
    }
    return std::get<E>(value_);
  }

  // Map function - transform Ok value, leave Err unchanged
  template <typename F>
  auto map(F&& f) -> Result<std::invoke_result_t<F, T>, E> {
    if (is_ok()) {
      return Result<std::invoke_result_t<F, T>, E>::Ok(f(std::get<T>(value_)));
    } else {
      return Result<std::invoke_result_t<F, T>, E>::Err(std::get<E>(value_));
    }
  }

  // Map error function - transform Err value, leave Ok unchanged
  template <typename F>
  auto map_err(F&& f) -> Result<T, std::invoke_result_t<F, E>> {
    using ReturnType = Result<T, std::invoke_result_t<F, E>>;
    if (is_err()) {
      return ReturnType::Err(f(std::get<E>(value_)));
    } else {
      return ReturnType::Ok(std::get<T>(value_));
    }
  }

  // And then - chain Results
  template <typename F>
  auto and_then(F&& f) -> std::invoke_result_t<F, T> {
    if (is_ok()) {
      return f(std::get<T>(value_));
    } else {
      using ReturnType = std::invoke_result_t<F, T>;
      return ReturnType::Err(std::get<E>(value_));
    }
  }

  // Pattern matching
  template <typename... Ts>
  auto match(Ts&&... ts) & -> decltype(auto) {
    return rust::match(value_, std::forward<Ts>(ts)...);
  }

  template <typename... Ts>
  auto match(Ts&&... ts) const& -> decltype(auto) {
    return rust::match(value_, std::forward<Ts>(ts)...);
  }

  template <typename... Ts>
  auto match(Ts&&... ts) && -> decltype(auto) {
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
  static Option Some(T value) { return Option(std::move(value)); }

  // Construct None option
  static Option None() { return Option(); }

  // Default constructor creates None
  Option() : value_(std::nullopt) {}

  ~Option() = default;

  // Check if option is Some
  bool is_some() const noexcept { return value_.has_value(); }

  // Check if option is None
  bool is_none() const noexcept { return !value_.has_value(); }

  // Get the Some value (throws if None)
  T& unwrap() & {
    if (is_none()) {
      throw std::runtime_error("Called unwrap() on a None Option");
    }
    return *value_;
  }

  const T& unwrap() const& {
    if (is_none()) {
      throw std::runtime_error("Called unwrap() on a None Option");
    }
    return *value_;
  }

  T&& unwrap() && {
    if (is_none()) {
      throw std::runtime_error("Called unwrap() on a None Option");
    }
    return std::move(*value_);
  }

  // Get the Some value or a default
  template <typename U>
  T unwrap_or(U&& default_value) const& {
    return is_some() ? *value_ : T(std::forward<U>(default_value));
  }

  template <typename U>
  T unwrap_or(U&& default_value) && {
    return is_some() ? std::move(*value_) : T(std::forward<U>(default_value));
  }

  // Map function - transform Some value, leave None unchanged
  template <typename F>
  auto map(F&& f) -> Option<std::invoke_result_t<F, T>> {
    using ReturnType = Option<std::invoke_result_t<F, T>>;
    if (is_some()) {
      return ReturnType::Some(f(*value_));
    } else {
      return ReturnType::None();
    }
  }

  // And then - chain Options
  template <typename F>
  auto and_then(F&& f) -> std::invoke_result_t<F, T> {
    if (is_some()) {
      return f(*value_);
    } else {
      using ReturnType = std::invoke_result_t<F, T>;
      return ReturnType::None();
    }
  }

  // Pattern matching using variant-like interface
  template <typename SomeFunc, typename NoneFunc>
  auto match(SomeFunc&& some_func, NoneFunc&& none_func) -> decltype(auto) {
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
#define INTERNAL_ENUM_VARIANT_WITH_FIELDS(name, ...)         \
  struct name {                                              \
    __VA_ARGS__;                                             \
    constexpr bool operator<=>(const name&) const = default; \
  }

// Helper macro for enum variants without fields.
// It takes only the name as an argument.
#define INTERNAL_ENUM_VARIANT_NO_FIELDS(name)                               \
  struct name {                                                             \
    constexpr bool operator==(const name&) const noexcept { return true; }  \
    constexpr bool operator!=(const name&) const noexcept { return false; } \
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
