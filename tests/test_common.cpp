#include <test_common.h>


void compare_vectors(const vec3 &v1, const vec3 &v2) {
  ASSERT_FLOAT_EQ(v1.x, v2.x) << debug_vector(v1, v2);
  ASSERT_FLOAT_EQ(v1.y, v2.y) << debug_vector(v1, v2);
  ASSERT_FLOAT_EQ(v1.z, v2.z) << debug_vector(v1, v2);
}
