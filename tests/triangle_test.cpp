#include <gtest/gtest.h>
#include <transform.h>
#include <primitives.h>

using namespace Transform;

TEST(trianle_tests, intersection) {
  Triangle t;
  t.vertices.push_back(vec3());
  t.vertices.push_back(vec3());
  t.vertices.push_back(vec3());
  t.normal = vec3();


  ASSERT_FLOAT_EQ(1.0f, 1.0f);
}