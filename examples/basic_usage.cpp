/*
 * Copyright (c) 2025 Dapeng Feng
 * All rights reserved.
 */

#include <iostream>
#include <string>

#include "rustcxx/rustcxx.hpp"

using namespace rust;  // NOLINT

// Example 1: Simple enum with variants

ENUM_VARIANT(Red);
ENUM_VARIANT(Green);
ENUM_VARIANT(Blue, int intensity = 0; int value = 0);  // NOLINT

using Color = Enum<Red, Green, Blue>;

// Example 2: More complex enum like Rust's Option or Result
ENUM_VARIANT(Loading);
ENUM_VARIANT(Success, std::string data);
ENUM_VARIANT(Error, std::string message);

using ApiResponse = Enum<Loading, Success, Error>;

// Example 3: Message passing enum
struct Connect {
  std::string address;
};
struct Send {
  std::string data;
};
ENUM_VARIANT(Disconnect);

using NetworkMessage = Enum<Connect, Send, Disconnect>;

void demonstrate_basic_enum() {
  std::cout << "=== Basic Enum Usage ===" << std::endl;

  const auto& visitor =
      overloads{[](const Red&) -> std::string { return "It's red!"; },
                [](const Green&) -> std::string { return "It's green!"; },
                [](const Blue& b) -> std::string {
                  return "It's blue with intensity " +
                         std::to_string(b.intensity) + std::to_string(b.value);
                }};

  Color color = Red{};

  // Pattern matching
  auto result = color.match(visitor);

  std::cout << result << std::endl;

  // Change the color
  color = Blue{128, 42};

  result = color.match(visitor);

  std::cout << result << std::endl;

  // Type checking
  if (color.is<Blue>()) {
    std::cout << "Color is blue with intensity: " << color.get<Blue>().intensity
              << std::endl;
  }
}

void demonstrate_api_response() {
  std::cout << "\n=== API Response Enum ===" << std::endl;

  ApiResponse response = Loading{};

  const auto& func = overloads{
      [](const Loading&) -> void { std::cout << "Loading..." << std::endl; },
      [](const Success& s) -> void {
        std::cout << "Success: " << s.data << std::endl;
      },
      [](const Error& e) -> void {
        std::cout << "Error: " << e.message << std::endl;
      }};

  // Handle different states
  response.match(func);

  // Simulate successful response
  response = Success{"Hello, World!"};

  response.match(func);

  // Simulate error
  response = Error{"Network timeout"};

  response.match(func);
}

void demonstrate_result_type() {
  std::cout << "\n=== Result Type Usage ===" << std::endl;

  auto divide = [](int a, int b) -> Result<double, std::string> {
    if (b == 0) {
      return Result<double, std::string>::Err("Division by zero!");
    }
    return Result<double, std::string>::Ok(static_cast<double>(a) / b);
  };

  auto result1 = divide(10, 2);
  auto result2 = divide(10, 0);

  const auto visitor = overloads{[](double value) -> void {
                                   std::cout << "Result: " << value
                                             << std::endl;
                                 },
                                 [](const std::string& error) -> void {
                                   std::cout << "Error: " << error << std::endl;
                                 }};

  // Pattern matching on results
  result1.match(visitor);

  result2.match(visitor);

  // Using unwrap_or for safe access
  std::cout << "Result with default: " << result2.unwrap_or(-1.0) << std::endl;

  // Chaining operations
  auto chained =
      divide(20, 4).map([](double x) { return x * 2; }).and_then([&](double x) {
        return divide(static_cast<int>(x), 2);
      });

  chained.match(
      [](double value) {
        std::cout << "Chained result: " << value << std::endl;
      },
      [](const std::string& error) {
        std::cout << "Chained error: " << error << std::endl;
      });
}

void demonstrate_option_type() {
  std::cout << "\n=== Option Type Usage ===" << std::endl;

  auto find_user = [](int id) -> Option<std::string> {
    if (id == 1) {
      return Option<std::string>::Some("Alice");
    } else if (id == 2) {
      return Option<std::string>::Some("Bob");
    }
    return Option<std::string>::None();
  };

  auto user1 = find_user(1);
  auto user2 = find_user(99);

  // Pattern matching on options
  user1.match(
      [](const std::string& name) {
        std::cout << "Found user: " << name << std::endl;
      },
      []() { std::cout << "User not found" << std::endl; });

  user2.match(
      [](const std::string& name) {
        std::cout << "Found user: " << name << std::endl;
      },
      []() { std::cout << "User not found" << std::endl; });

  // Using unwrap_or for safe access
  std::cout << "User or default: " << user2.unwrap_or("Unknown") << std::endl;

  // Chaining operations
  auto processed =
      find_user(1)
          .map([](const std::string& name) { return "Hello, " + name + "!"; })
          .and_then([](const std::string& greeting) -> Option<std::string> {
            if (greeting.length() > 5) {
              return Option<std::string>::Some(greeting);
            }
            return Option<std::string>::None();
          });

  processed.match(
      [](const std::string& msg) {
        std::cout << "Processed: " << msg << std::endl;
      },
      []() { std::cout << "Processing failed" << std::endl; });
}

int main() {
  demonstrate_basic_enum();
  demonstrate_api_response();
  demonstrate_result_type();
  demonstrate_option_type();

  return 0;
}
