/*
 * Copyright (c) 2025 Dapeng Feng
 * All rights reserved.
 */

#include <gtest/gtest.h>

#include <string>

#include "rustcxx/rustcxx.hpp"

using namespace rust;  // NOLINT

class ResultTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(ResultTest, OkConstruction) {
  auto result = Result<int, std::string>::Ok(42);

  EXPECT_TRUE(result.is_ok());
  EXPECT_FALSE(result.is_err());
  EXPECT_EQ(result.unwrap(), 42);
}

TEST_F(ResultTest, ErrConstruction) {
  auto result = Result<int, std::string>::Err("Something went wrong");

  EXPECT_FALSE(result.is_ok());
  EXPECT_TRUE(result.is_err());
  EXPECT_EQ(result.unwrap_err(), "Something went wrong");
}

TEST_F(ResultTest, UnwrapOr) {
  auto ok_result = Result<int, std::string>::Ok(42);
  auto err_result = Result<int, std::string>::Err("Error");

  EXPECT_EQ(ok_result.unwrap_or(-1), 42);
  EXPECT_EQ(err_result.unwrap_or(-1), -1);
}

TEST_F(ResultTest, PatternMatching) {
  auto ok_result = Result<int, std::string>::Ok(42);
  auto err_result = Result<int, std::string>::Err("Error message");

  std::string ok_msg = ok_result.match(
      [](int value) { return "Got value: " + std::to_string(value); },
      [](const std::string& error) { return "Got error: " + error; });

  std::string err_msg = err_result.match(
      [](int value) { return "Got value: " + std::to_string(value); },
      [](const std::string& error) { return "Got error: " + error; });

  EXPECT_EQ(ok_msg, "Got value: 42");
  EXPECT_EQ(err_msg, "Got error: Error message");
}

TEST_F(ResultTest, Map) {
  auto result = Result<int, std::string>::Ok(21);

  auto doubled = result.map([](int x) { return x * 2; });

  EXPECT_TRUE(doubled.is_ok());
  EXPECT_EQ(doubled.unwrap(), 42);

  // Map on error should pass through
  auto err_result = Result<int, std::string>::Err("Error");
  auto mapped_err = err_result.map([](int x) { return x * 2; });

  EXPECT_TRUE(mapped_err.is_err());
  EXPECT_EQ(mapped_err.unwrap_err(), "Error");
}

TEST_F(ResultTest, MapErr) {
  auto result = Result<int, std::string>::Err("original error");

  auto mapped =
      result.map_err([](const std::string& err) { return "Mapped: " + err; });

  EXPECT_TRUE(mapped.is_err());
  EXPECT_EQ(mapped.unwrap_err(), "Mapped: original error");

  // Map error on Ok should pass through
  auto ok_result = Result<int, std::string>::Ok(42);
  auto mapped_ok = ok_result.map_err(
      [](const std::string& err) { return "Mapped: " + err; });

  EXPECT_TRUE(mapped_ok.is_ok());
  EXPECT_EQ(mapped_ok.unwrap(), 42);
}

TEST_F(ResultTest, AndThen) {
  auto divide = [](int a, int b) -> Result<double, std::string> {
    if (b == 0) {
      return Result<double, std::string>::Err("Division by zero");
    }
    return Result<double, std::string>::Ok(static_cast<double>(a) / b);
  };

  auto result = Result<int, std::string>::Ok(10);

  auto chained = result.and_then([&](int x) { return divide(x, 2); });

  EXPECT_TRUE(chained.is_ok());
  EXPECT_EQ(chained.unwrap(), 5.0);

  // Chain with error
  auto chained_err = result.and_then([&](int x) { return divide(x, 0); });

  EXPECT_TRUE(chained_err.is_err());
  EXPECT_EQ(chained_err.unwrap_err(), "Division by zero");

  // Chain starting with error
  auto err_result = Result<int, std::string>::Err("Initial error");
  auto chained_from_err =
      err_result.and_then([&](int x) { return divide(x, 2); });

  EXPECT_TRUE(chained_from_err.is_err());
  EXPECT_EQ(chained_from_err.unwrap_err(), "Initial error");
}

TEST_F(ResultTest, UnwrapThrows) {
  auto err_result = Result<int, std::string>::Err("Error");

  EXPECT_THROW(err_result.unwrap(), std::runtime_error);

  auto ok_result = Result<int, std::string>::Ok(42);

  EXPECT_THROW(ok_result.unwrap_err(), std::runtime_error);
}
