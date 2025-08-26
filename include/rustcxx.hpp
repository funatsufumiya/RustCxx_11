/*
 * Copyright (c) 2025 Dapeng Feng
 * All rights reserved.
 */

#pragma once
#include <functional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <optional.hpp> // optional-lite
#include <variant.hpp>  // variant-lite

namespace rust {

// Helper struct for creating overloaded visitors (C++17 compatible)
template <class... Ts>
struct overloads : Ts... {
  using Ts::operator()...;
};

// Helper for pattern matching - similar to Rust's match
template <typename Variant, class... Ts>
inline constexpr decltype(auto) match(Variant&& variant, Ts&&... ts) noexcept {
  return nonstd::visit(overloads<Ts...>{std::forward<Ts>(ts)...}, std::forward<Variant>(variant));
}

// Enum-like wrapper around std::variant for better ergonomics
template <typename... Types>
class Enum {
 public:
  // Default constructor
  Enum() {}

  // Constructor from any of the variant types
  template <typename T>
  Enum(T&& t) : value_(std::forward<T>(t)) {}

  // Assignment
  template <typename T>
  Enum& operator=(T&& t) {
    value_ = std::forward<T>(t);
    return *this;
  }

  ~Enum() {}

  // Get the index of the currently held type
  inline std::size_t index() const { return value_.index(); }

  // Check if the enum holds a specific type
  template <typename T>
  inline bool is() const {
    return nonstd::holds_alternative<T>(value_);
  }

  // Get the value if it's of type T, throws if not
  template <typename T>
  inline T& get() {
    if (!is<T>()) {
      throw std::runtime_error("bad variant access");
    }
    return nonstd::get<T>(value_);
  }

  template <typename T>
  inline const T& get() const {
    if (!is<T>()) {
      throw std::runtime_error("bad variant access");
    }
    return nonstd::get<T>(value_);
  }

  // Get the value if it's of type T, returns nullptr if not
  template <typename T>
  inline T* get_if() {
    if (!is<T>()) {
      return NULL;
    }
    return nonstd::get_if<T>(&value_);
  }

  template <typename T>
  inline const T* get_if() const {
    if (!is<T>()) {
      return NULL;
    }
    return nonstd::get_if<T>(&value_);
  }

  // Match function for pattern matching
  template <typename... Ts>
  inline constexpr decltype(auto) match(Ts&&... ts) & noexcept {
    return rust::match(value_, std::forward<Ts>(ts)...);
  }

  template <typename... Ts>
  inline constexpr decltype(auto) match(Ts&&... ts) const& noexcept {
    return rust::match(value_, std::forward<Ts>(ts)...);
  }

  template <typename... Ts>
  inline constexpr decltype(auto) match(Ts&&... ts) && noexcept {
    return rust::match(std::move(value_), std::forward<Ts>(ts)...);
  }

  // Equality comparison
  inline bool operator==(const Enum& other) const {
    return value_ == other.value_;
  }

  inline bool operator!=(const Enum& other) const {
    return value_ != other.value_;
  }

 private:
  nonstd::variant<Types...> value_;
};

// Rust-style Result type
template <typename T, typename E = std::string>
class Result {
 public:
  // Construct Ok result
  static Result Ok(T value) {
    return Result(value);
  }

  // Construct Err result
  static Result Err(E error) {
    return Result(error);
  }

  ~Result() {}

  // Check if result is Ok
  bool is_ok() const {
    return nonstd::holds_alternative<T>(value_);
  }

  // Check if result is Err
  bool is_err() const {
    return nonstd::holds_alternative<E>(value_);
  }

  // Get the Ok value (throws if Err)
  T& unwrap() {
    if (is_err()) {
      throw std::runtime_error("Called unwrap() on an Err Result");
    }
    return nonstd::get<T>(value_);
  }

  const T& unwrap() const {
    if (is_err()) {
      throw std::runtime_error("Called unwrap() on an Err Result");
    }
    return nonstd::get<T>(value_);
  }

  // Get the Ok value or a default
  template <typename U>
  T unwrap_or(U&& default_value) const {
    return is_ok() ? nonstd::get<T>(value_) : T(std::forward<U>(default_value));
  }

  // Get the error value (throws if Ok)
  E& unwrap_err() {
    if (is_ok()) {
      throw std::runtime_error("Called unwrap_err() on an Ok Result");
    }
    return nonstd::get<E>(value_);
  }

  const E& unwrap_err() const {
    if (is_ok()) {
      throw std::runtime_error("Called unwrap_err() on an Ok Result");
    }
    return nonstd::get<E>(value_);
  }

  // Map function - transform Ok value, leave Err unchanged
  template <typename F>
  auto map(F&& f) -> Result<decltype(f(std::declval<T>())), E> {
    typedef Result<decltype(f(std::declval<T>())), E> result;
    if (is_ok()) {
      return result::Ok(f(nonstd::get<T>(value_)));
    } else {
      return result::Err(nonstd::get<E>(value_));
    }
  }

  // Map error function - transform Err value, leave Ok unchanged
  template <typename F>
  auto map_err(F&& f) -> Result<T, decltype(f(std::declval<E>()))> {
    typedef Result<T, decltype(f(std::declval<E>()))> result;
    if (is_err()) {
      return result::Err(f(nonstd::get<E>(value_)));
    } else {
      return result::Ok(nonstd::get<T>(value_));
    }
  }

  // And then - chain Results
  template <typename F>
  auto and_then(F&& f) -> decltype(f(std::declval<T>())) {
    if (is_ok()) {
      return f(nonstd::get<T>(value_));
    } else {
      typedef decltype(f(std::declval<T>())) result;
      return result::Err(nonstd::get<E>(value_));
    }
  }

  // Pattern matching
  template <typename... Ts>
  auto match(Ts&&... ts) {
    return rust::match(value_, std::forward<Ts>(ts)...);
  }

  template <typename... Ts>
  auto match(Ts&&... ts) const {
    return rust::match(value_, std::forward<Ts>(ts)...);
  }

 private:
  template <typename U>
  explicit Result(U&& u) : value_(std::forward<U>(u)) {}

  nonstd::variant<T, E> value_;
};

// Rust-style Option type
template <typename T>
class Option {
 public:
  // Construct Some option
  static Option Some(T value) {
    return Option(value);
  }

  // Construct None option
  static Option None() {
    return Option();
  }

  // Default constructor creates None
  Option() : value_() {}

  ~Option() {}

  // Check if option is Some
  bool is_some() const { return value_.has_value(); }

  // Check if option is None
  bool is_none() const { return !value_.has_value(); }

  // Get the Some value (throws if None)
  T& unwrap() {
    if (is_none()) {
      throw std::runtime_error("Called unwrap() on a None Option");
    }
    return *value_;
  }

  const T& unwrap() const {
    if (is_none()) {
      throw std::runtime_error("Called unwrap() on a None Option");
    }
    return *value_;
  }

  // Get the Some value or a default
  template <typename U>
  T unwrap_or(U&& default_value) const {
    return is_some() ? *value_ : T(std::forward<U>(default_value));
  }

  // Map function - transform Some value, leave None unchanged
  template <typename F>
  auto map(F&& f) -> Option<decltype(f(std::declval<T>()))> {
    typedef Option<decltype(f(std::declval<T>()))> option;
    if (is_some()) {
      return option::Some(f(*value_));
    } else {
      return option::None();
    }
  }

  // And then - chain Options
  template <typename F>
  auto and_then(F&& f) -> decltype(f(std::declval<T>())) {
    if (is_some()) {
      return f(*value_);
    } else {
      typedef decltype(f(std::declval<T>())) option;
      return option::None();
    }
  }

  // Pattern matching using variant-like interface
  template <typename SomeFunc, typename NoneFunc>
  auto match(SomeFunc&& some_func, NoneFunc&& none_func) {
    if (is_some()) {
      return some_func(*value_);
    } else {
      return none_func();
    }
  }

 private:
  explicit Option(T value) : value_(value) {}

  nonstd::optional<T> value_;
};

}  // namespace rust


// ENUM_VARIANT: original macro supporting both no-field and field variants (for compatibility)
#define INTERNAL_ENUM_VARIANT_WITH_FIELDS(name, ...)                         \
  struct name {                                                              \
    __VA_ARGS__;                                                             \
    bool operator==(const name& other) const { return true; }                \
    bool operator!=(const name& other) const { return false; }               \
  }

#define INTERNAL_ENUM_VARIANT_NO_FIELDS(name)                      \
  struct name {                                                    \
    inline constexpr bool operator==(const name&) const noexcept { \
      return true;                                                 \
    }                                                              \
    inline constexpr bool operator!=(const name&) const noexcept { \
      return false;                                                \
    }                                                              \
  }

#define GET_DISPATCH_MACRO(_1, _2, NAME, ...) NAME

#define ENUM_VARIANT(...)                                            \
  GET_DISPATCH_MACRO(__VA_ARGS__, INTERNAL_ENUM_VARIANT_WITH_FIELDS, \
                     INTERNAL_ENUM_VARIANT_NO_FIELDS)(__VA_ARGS__)

// ENUM_VARIANT0: no fields
#define ENUM_VARIANT0(name) \
  struct name { \
    name() = default; \
    ~name() = default; \
    bool operator==(const name&) const { return true; } \
    bool operator!=(const name&) const { return false; } \
  }

// ENUM_VARIANT1: 1 field (type, field)
#define ENUM_VARIANT1(name, type1, field1) \
  struct name { \
    type1 field1; \
    name() = default; \
    ~name() = default; \
    explicit name(type1 v1) : field1(v1) {} \
    bool operator==(const name&) const { return true; } \
    bool operator!=(const name&) const { return false; } \
  }

// ENUM_VARIANT2: 2 fields (type, field, type, field)
#define ENUM_VARIANT2(name, type1, field1, type2, field2) \
  struct name { \
    type1 field1; \
    type2 field2; \
    name() = default; \
    ~name() = default; \
    name(type1 v1, type2 v2) : field1(v1), field2(v2) {} \
    bool operator==(const name&) const { return true; } \
    bool operator!=(const name&) const { return false; } \
  }

// ENUM_VARIANT3: 3 fields (type, field, ...)
#define ENUM_VARIANT3(name, type1, field1, type2, field2, type3, field3) \
  struct name { \
    type1 field1; \
    type2 field2; \
    type3 field3; \
    name() = default; \
    ~name() = default; \
    name(type1 v1, type2 v2, type3 v3) : field1(v1), field2(v2), field3(v3) {} \
    bool operator==(const name&) const { return true; } \
    bool operator!=(const name&) const { return false; } \
  }

// ENUM_VARIANT4: 4 fields (type, field, ...)
#define ENUM_VARIANT4(name, type1, field1, type2, field2, type3, field3, type4, field4) \
  struct name { \
    type1 field1; \
    type2 field2; \
    type3 field3; \
    type4 field4; \
    name() = default; \
    ~name() = default; \
    name(type1 v1, type2 v2, type3 v3, type4 v4) : field1(v1), field2(v2), field3(v3), field4(v4) {} \
    bool operator==(const name&) const { return true; } \
    bool operator!=(const name&) const { return false; } \
  }

// ENUM_VARIANT5: 5 fields (type, field, ...)
#define ENUM_VARIANT5(name, type1, field1, type2, field2, type3, field3, type4, field4, type5, field5) \
  struct name { \
    type1 field1; \
    type2 field2; \
    type3 field3; \
    type4 field4; \
    type5 field5; \
    name() = default; \
    ~name() = default; \
    name(type1 v1, type2 v2, type3 v3, type4 v4, type5 v5) : field1(v1), field2(v2), field3(v3), field4(v4), field5(v5) {} \
    bool operator==(const name&) const { return true; } \
    bool operator!=(const name&) const { return false; } \
  }

// ENUM_VARIANT6: 6 fields (type, field, ...)
#define ENUM_VARIANT6(name, type1, field1, type2, field2, type3, field3, type4, field4, type5, field5, type6, field6) \
  struct name { \
    type1 field1; \
    type2 field2; \
    type3 field3; \
    type4 field4; \
    type5 field5; \
    type6 field6; \
    name() = default; \
    ~name() = default; \
    name(type1 v1, type2 v2, type3 v3, type4 v4, type5 v5, type6 v6) : field1(v1), field2(v2), field3(v3), field4(v4), field5(v5), field6(v6) {} \
    bool operator==(const name&) const { return true; } \
    bool operator!=(const name&) const { return false; } \
  }

// ENUM_VARIANT7: 7 fields (type, field, ...)
#define ENUM_VARIANT7(name, type1, field1, type2, field2, type3, field3, type4, field4, type5, field5, type6, field6, type7, field7) \
  struct name { \
    type1 field1; \
    type2 field2; \
    type3 field3; \
    type4 field4; \
    type5 field5; \
    type6 field6; \
    type7 field7; \
    name() = default; \
    ~name() = default; \
    name(type1 v1, type2 v2, type3 v3, type4 v4, type5 v5, type6 v6, type7 v7) : field1(v1), field2(v2), field3(v3), field4(v4), field5(v5), field6(v6), field7(v7) {} \
    bool operator==(const name&) const { return true; } \
    bool operator!=(const name&) const { return false; } \
  }
