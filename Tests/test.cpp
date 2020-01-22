#include <gtest/gtest.h>
#include <transform.h>

using namespace Transform;

std::string debug_matrix(const mat4 &t) {
  std::ostringstream ss;
  ss << std::setw(4) << t[0][0] << " " << std::setw(4) << t[1][0] << " "
     << std::setw(4) << t[2][0] << std::endl;
  ss << std::setw(4) << t[0][1] << " " << std::setw(4) << t[1][1] << " "
     << std::setw(4) << t[2][1] << std::endl;
  ss << std::setw(4) << t[0][2] << " " << std::setw(4) << t[1][2] << " "
     << std::setw(4) << t[2][2] << std::endl;

  return ss.str();
}

void compare_vectors(const vec3 &v1, const vec3 &v2) {
  ASSERT_FLOAT_EQ(v1.x, v2.x);
  ASSERT_FLOAT_EQ(v1.y, v2.y);
  ASSERT_FLOAT_EQ(v1.z, v2.z);
}

void compare_matrices(const mat3 &m1, const mat3 &m2) {
  for (size_t i = 0; i < 9; ++i) {
    size_t row = i % 3, col = i / 3;
    ASSERT_FLOAT_EQ(m1[col][row], m2[col][row])
        << "col: " << col << " row: " << row << std::endl
        << debug_matrix(m1);
  }
}

TEST(test_group_1, radians) { ASSERT_FLOAT_EQ(radians(1), 0.0174533f); }

TEST(test_group_1, length_1) {
  ASSERT_FLOAT_EQ(length(vec3(1.f, 0.f, 0.f)), 1.0f);
}

TEST(test_group_1, length_2) {
  ASSERT_FLOAT_EQ(length(vec3(1.f, 1.f, 1.f)), 1.7320508f);
}

TEST(test_group_1, normalize) {
  vec3 v = normalize(vec3(1.f, 1.f, 1.f));
  compare_vectors(v, vec3(0.57735026f, 0.57735026f, 0.57735026f));
}

TEST(test_group_1, dot_product_1) {
  float parallel = dot(vec3(1.f, 0.f, 0.f), vec3(1.f, 0.f, 0.f));
  ASSERT_FLOAT_EQ(parallel, 1.0f);
}

TEST(test_group_1, dot_product_2) {
  float perpendicular = dot(vec3(1.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
  ASSERT_FLOAT_EQ(perpendicular, 0.0f);
}

TEST(test_group_1, cross_product) {
  vec3 z = cross(vec3(1.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
  compare_vectors(z, vec3(0.0f, 0.0f, 1.0f));
}

TEST(test_group_1, reflect) {
  vec3 reflected = reflect(vec3(1.f, -1.f, 0.f), vec3(0.f, 1.f, 0.f));
  compare_vectors(reflected, vec3(1.0f, 1.0f, 0.0f));
}

TEST(test_group_1, transpose_1) {
  compare_matrices(transpose(mat3(1.f)), mat3(1.f));
}

TEST(test_group_1, transpose_2) {
  mat3 transposed = transpose(mat3(1.f, 1.f, 1.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f));
  compare_matrices(transposed, mat3(1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 1.f, 0.f, 1.f));
}

TEST(test_group_1, transpose_3) {
  compare_matrices(transpose(transpose(mat3(1.f))), mat3(1.f));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}