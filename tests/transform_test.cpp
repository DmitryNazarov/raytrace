#include <gtest/gtest.h>
#include <transform.h>
#include <sstream>

using namespace Transform;

template<int N>
std::string debug_matrix(const mat<N> &t) {
  std::ostringstream ss;
  for (int i = 0; i < N; ++i) {
    ss << std::setw(4);
    for (int j = 0; j < N; ++j) {
      ss << t[i][j] << " ";
    }
    ss << std::endl;
  }

  return ss.str();
}

void compare_vectors(const vec3 &v1, const vec3 &v2) {
  ASSERT_FLOAT_EQ(v1.x, v2.x);
  ASSERT_FLOAT_EQ(v1.y, v2.y);
  ASSERT_FLOAT_EQ(v1.z, v2.z);
}

template <int N>
void compare_matrices(const mat<N> &m1, const mat<N> &m2) {
  for (int i = 0; i < N * N; ++i) {
    int row = i % N, col = i / N;
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

TEST(test_group_1, create_submatrix) {
  mat3 m(2.f, 6.f, 5.f, 5.f, 3.f, -2.f, 7.f, 4.f, -3.f);
  mat2 sub_m = create_submatrix(m, 1, 1);
  debug_matrix(sub_m);
  compare_matrices(sub_m, mat2(2.f, 5.f, 7.f, -3.f));
}

TEST(test_group_1, determinant) {
  mat3 m(2.f, 6.f, 5.f, 5.f, 3.f, -2.f, 7.f, 4.f, -3.f);
  ASSERT_FLOAT_EQ(determinant(m), -1.f);
}
