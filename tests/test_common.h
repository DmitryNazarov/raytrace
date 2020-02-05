#include <gtest/gtest.h>
#include <transform.h>

using namespace Transform;


void compare_vectors(const vec3 &v1, const vec3 &v2);

template <int N>
void compare_matrices(const mat<N> &m1, const mat<N> &m2) {
  for (int i = 0; i < N * N; ++i) {
    int row = i % N, col = i / N;
    ASSERT_FLOAT_EQ(m1[col][row], m2[col][row])
        << "col: " << col << " row: " << row << "\n"
        << debug_matrix(m1);
  }
}
