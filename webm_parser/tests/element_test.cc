#include "webm/element.h"

#include <string>
#include <utility>

#include "gtest/gtest.h"

using webm::Element;

namespace {

class ElementTest : public testing::Test {};

TEST_F(ElementTest, Construction) {
  Element<int> value_initialized;
  EXPECT_EQ(false, value_initialized.is_present());
  EXPECT_EQ(0, value_initialized.value());

  Element<int> absent_custom_default(1);
  EXPECT_EQ(false, absent_custom_default.is_present());
  EXPECT_EQ(1, absent_custom_default.value());

  Element<int> present(2, true);
  EXPECT_EQ(true, present.is_present());
  EXPECT_EQ(2, present.value());
}

TEST_F(ElementTest, Assignment) {
  Element<int> e;

  e.Set(42, true);
  EXPECT_EQ(true, e.is_present());
  EXPECT_EQ(42, e.value());

  *e.mutable_value() = 0;
  EXPECT_EQ(true, e.is_present());
  EXPECT_EQ(0, e.value());
}

}  // namespace
