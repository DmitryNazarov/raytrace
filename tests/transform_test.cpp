#include <test_common.h>


TEST(math_tests, radians) { ASSERT_FLOAT_EQ(radians(1), 0.0174533f); }

TEST(math_tests, length_1) {
  ASSERT_FLOAT_EQ(length(vec3(1.f, 0.f, 0.f)), 1.0f);
}

TEST(math_tests, length_2) {
  ASSERT_FLOAT_EQ(length(vec3(1.f, 1.f, 1.f)), 1.7320508f);
}

TEST(math_tests, normalize) {
  vec3 v = normalize(vec3(1.f, 1.f, 1.f));
  compare_vectors(v, vec3(0.57735026f, 0.57735026f, 0.57735026f));
}

TEST(math_tests, dot_product_1) {
  float parallel = dot(vec3(1.f, 0.f, 0.f), vec3(1.f, 0.f, 0.f));
  ASSERT_FLOAT_EQ(parallel, 1.0f);
}

TEST(math_tests, dot_product_2) {
  float perpendicular = dot(vec3(1.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
  ASSERT_FLOAT_EQ(perpendicular, 0.0f);
}

TEST(math_tests, cross_product) {
  vec3 z = cross(vec3(1.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
  compare_vectors(z, vec3(0.0f, 0.0f, 1.0f));
}

TEST(math_tests, reflect) {
  vec3 reflected = reflect(vec3(1.f, -1.f, 0.f), vec3(0.f, 1.f, 0.f));
  compare_vectors(reflected, vec3(1.0f, 1.0f, 0.0f));
}

TEST(math_tests, transpose_1) {
  compare_matrices(transpose(mat3(1.f)), mat3(1.f));
}

TEST(math_tests, transpose_2) {
  mat3 transposed = transpose(mat3(1.f, 1.f, 1.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f));
  compare_matrices(transposed, mat3(1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 1.f, 0.f, 1.f));
}

TEST(math_tests, transpose_3) {
  compare_matrices(transpose(transpose(mat3(1.f))), mat3(1.f));
}

TEST(math_tests, create_submatrix) {
  mat3 m(2.f, 6.f, 5.f, 5.f, 3.f, -2.f, 7.f, 4.f, -3.f);
  mat2 sub_m = create_submatrix(m, 1, 1);
  compare_matrices(sub_m, mat2(2.f, 5.f, 7.f, -3.f));
}

TEST(math_tests, determinant_1) {
  mat2 m(2.f, 6.f, 5.f, 5.f);
  ASSERT_FLOAT_EQ(determinant(m), -20.f);
}

TEST(math_tests, determinant_2) {
  mat3 m(2.f, 6.f, 5.f, 5.f, 3.f, -2.f, 7.f, 4.f, -3.f);
  ASSERT_FLOAT_EQ(determinant(m), -1.f);
}

TEST(math_tests, determinant_3) {
  mat4 m(2.f, 1.f, 3.f, 4.f, 1.f, 3.f, 4.f, 2.f, 3.f, 4.f, 2.f, 1.f, 4.f, 2.f, 1.f, 3.f);
  ASSERT_FLOAT_EQ(determinant(m), 0.f);
}

TEST(math_tests, inverse_1) {
  mat3 m(2.f, 6.f, 5.f, 5.f, 3.f, -2.f, 7.f, 4.f, -3.f);
  mat3 inv_m(1.f, -38.f, 27.f, -1.f, 41.f, -29.f, 1.f, -34.f, 24.f);
  compare_matrices(inverse(m), inv_m);
}

TEST(math_tests, inverse_2) {
  mat4 m(0.707107f, -0.707107f, 0.f, 0.f, 0.176777f, 0.176777f, 0.f, 0.f, 0.f, 0.f, 0.25f, 0.f, 0.f, 0.f, 0.5f, 1.f);
  compare_matrices(inverse(m) * m, mat4(1.f));
}
