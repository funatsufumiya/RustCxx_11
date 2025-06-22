/*
 * Copyright (c) 2025 Dapeng Feng
 * All rights reserved.
 */

#include <gtest/gtest.h>

#include <string>

#include "rustcxx/rustcxx.hpp"

using namespace rust;  // NOLINT

class OptionTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(OptionTest, SomeConstruction) {
  auto option = Option<int>::Some(42);

  EXPECT_TRUE(option.is_some());
  EXPECT_FALSE(option.is_none());
  EXPECT_EQ(option.unwrap(), 42);
}

TEST_F(OptionTest, NoneConstruction) {
  auto option = Option<int>::None();

  EXPECT_FALSE(option.is_some());
  EXPECT_TRUE(option.is_none());
}

TEST_F(OptionTest, DefaultConstruction) {
  Option<int> option;

  EXPECT_FALSE(option.is_some());
  EXPECT_TRUE(option.is_none());
}

TEST_F(OptionTest, UnwrapOr) {
  auto some_option = Option<int>::Some(42);
  auto none_option = Option<int>::None();

  EXPECT_EQ(some_option.unwrap_or(-1), 42);
  EXPECT_EQ(none_option.unwrap_or(-1), -1);
}

TEST_F(OptionTest, PatternMatching) {
  auto some_option = Option<std::string>::Some("Hello");
  auto none_option = Option<std::string>::None();

  std::string some_msg = some_option.match(
      [](const std::string& value) { return "Got: " + value; },
      []() { return std::string("Got nothing"); });

  std::string none_msg = none_option.match(
      [](const std::string& value) { return "Got: " + value; },
      []() { return std::string("Got nothing"); });

  EXPECT_EQ(some_msg, "Got: Hello");
  EXPECT_EQ(none_msg, "Got nothing");
}

TEST_F(OptionTest, Map) {
  auto option = Option<int>::Some(21);

  auto doubled = option.map([](int x) { return x * 2; });

  EXPECT_TRUE(doubled.is_some());
  EXPECT_EQ(doubled.unwrap(), 42);

  // Map on None should remain None
  auto none_option = Option<int>::None();
  auto mapped_none = none_option.map([](int x) { return x * 2; });

  EXPECT_TRUE(mapped_none.is_none());
}

TEST_F(OptionTest, AndThen) {
  auto safe_divide = [](int a, int b) -> Option<double> {
    if (b == 0) {
      return Option<double>::None();
    }
    return Option<double>::Some(static_cast<double>(a) / b);
  };

  auto option = Option<int>::Some(10);

  auto chained = option.and_then([&](int x) { return safe_divide(x, 2); });

  EXPECT_TRUE(chained.is_some());
  EXPECT_EQ(chained.unwrap(), 5.0);

  // Chain with operation that returns None
  auto chained_none = option.and_then([&](int x) { return safe_divide(x, 0); });

  EXPECT_TRUE(chained_none.is_none());

  // Chain starting with None
  auto none_option = Option<int>::None();
  auto chained_from_none =
      none_option.and_then([&](int x) { return safe_divide(x, 2); });

  EXPECT_TRUE(chained_from_none.is_none());
}

TEST_F(OptionTest, UnwrapThrows) {
  auto none_option = Option<int>::None();

  EXPECT_THROW(none_option.unwrap(), std::runtime_error);
}

TEST_F(OptionTest, ChainedOperations) {
  auto parse_int = [](const std::string& s) -> Option<int> {
    try {
      return Option<int>::Some(std::stoi(s));
    } catch (...) {
      return Option<int>::None();
    }
  };

  auto double_if_positive = [](int x) -> Option<int> {
    if (x > 0) {
      return Option<int>::Some(x * 2);
    }
    return Option<int>::None();
  };

  // Successful chain
  auto result1 = parse_int("42").and_then(double_if_positive);

  EXPECT_TRUE(result1.is_some());
  EXPECT_EQ(result1.unwrap(), 84);

  // Failed parse
  auto result2 = parse_int("not_a_number").and_then(double_if_positive);

  EXPECT_TRUE(result2.is_none());

  // Successful parse but fails condition
  auto result3 = parse_int("-5").and_then(double_if_positive);

  EXPECT_TRUE(result3.is_none());
}
