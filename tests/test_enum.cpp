/*
 * Copyright (c) 2025 Dapeng Feng
 * All rights reserved.
 */

#include <gtest/gtest.h>

#include <string>

#include "rustcxx/rustcxx.hpp"

using namespace rust;  // NOLINT

// Test variants
ENUM_VARIANT(Red);
ENUM_VARIANT(Green);
ENUM_VARIANT(Blue, int intensity = 0);  // NOLINT

using Color = Enum<Red, Green, Blue>;

// Message enum for testing
ENUM_VARIANT(TextMessage, std::string content; int data);  // NOLINT
ENUM_VARIANT(NumberMessage, int value);                    // NOLINT
ENUM_VARIANT(EmptyMessage);

using Message = Enum<TextMessage, NumberMessage, EmptyMessage>;

class EnumTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(EnumTest, BasicConstruction) {
  Color red = Red{};
  Color green = Green{};
  Color blue = Blue{255};

  EXPECT_TRUE(red.is<Red>());
  EXPECT_FALSE(red.is<Green>());
  EXPECT_FALSE(red.is<Blue>());

  EXPECT_TRUE(green.is<Green>());
  EXPECT_FALSE(green.is<Red>());
  EXPECT_FALSE(green.is<Blue>());

  EXPECT_TRUE(blue.is<Blue>());
  EXPECT_FALSE(blue.is<Red>());
  EXPECT_FALSE(blue.is<Green>());
}

TEST_F(EnumTest, GetValue) {
  Color blue = Blue{128};

  EXPECT_EQ(blue.get<Blue>().intensity, 128);

  auto* blue_ptr = blue.get_if<Blue>();
  ASSERT_NE(blue_ptr, nullptr);
  EXPECT_EQ(blue_ptr->intensity, 128);

  auto* red_ptr = blue.get_if<Red>();
  EXPECT_EQ(red_ptr, nullptr);
}

TEST_F(EnumTest, PatternMatching) {
  Color blue = Blue{200};

  const auto& visitor = overloads{
      [](const Red&) { return std::string("red"); },
      [](const Green&) { return std::string("green"); },
      [](const Blue& b) { return "blue:" + std::to_string(b.intensity); },
  };

  auto result = blue.match(visitor);

  EXPECT_EQ(result, "blue:200");

  Color red = Red{};
  result = red.match(visitor);

  EXPECT_EQ(result, "red");
}

TEST_F(EnumTest, Assignment) {
  Color color = Red{};
  EXPECT_TRUE(color.is<Red>());

  color = Green{};
  EXPECT_TRUE(color.is<Green>());
  EXPECT_FALSE(color.is<Red>());

  color = Blue{42};
  EXPECT_TRUE(color.is<Blue>());
  EXPECT_EQ(color.get<Blue>().intensity, 42);
}

TEST_F(EnumTest, Equality) {
  Color red1 = Red{};
  Color red2 = Red{};
  Color green = Green{};
  Color blue1 = Blue{100};
  Color blue2 = Blue{100};
  Color blue3 = Blue{200};

  EXPECT_EQ(red1, red2);
  EXPECT_NE(red1, green);
  EXPECT_NE(red1, blue1);

  EXPECT_EQ(blue1, blue2);
  EXPECT_NE(blue1, blue3);
}

TEST_F(EnumTest, ComplexData) {
  Message text_msg = TextMessage{"Hello, World!", 123};
  Message num_msg = NumberMessage{42};
  Message empty_msg = EmptyMessage{};

  EXPECT_TRUE(text_msg.is<TextMessage>());
  EXPECT_EQ(text_msg.get<TextMessage>().content, "Hello, World!");

  EXPECT_TRUE(num_msg.is<NumberMessage>());
  EXPECT_EQ(num_msg.get<NumberMessage>().value, 42);

  EXPECT_TRUE(empty_msg.is<EmptyMessage>());
}

TEST_F(EnumTest, Index) {
  Color red = Red{};
  Color green = Green{};
  Color blue = Blue{100};

  // Index depends on order in template parameters
  EXPECT_NE(red.index(), green.index());
  EXPECT_NE(green.index(), blue.index());
  EXPECT_NE(red.index(), blue.index());
}
