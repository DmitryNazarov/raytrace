#include <test_common.h>


void compare_vectors(const vec3 &v1, const vec3 &v2) {
  ASSERT_NEAR(v1.x, v2.x, 0.01) << debug_vector(v1, v2);
  ASSERT_NEAR(v1.y, v2.y, 0.01) << debug_vector(v1, v2);
  ASSERT_NEAR(v1.z, v2.z, 0.01) << debug_vector(v1, v2);
}
