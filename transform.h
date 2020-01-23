#include <type_traits>
#define _USE_MATH_DEFINES
#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Transform {

// template <auto N, class _type = decltype(N),
//           class = typename
//           std::enable_if<std::is_signed<_type>::value>::type>
// class vec {
// public:
//   vec() {}

//   template <class... T> vec(T... args) {
//     size_t i = 0;
//     for (const auto p : {args...}) {
//       values[i++] = p;
//     }
//   }

//   vec(const std::array<_type, N> &v) {
//     for (size_t i = 0; i < N; ++i) {
//       values[i] = v[i];
//     }
//   }

//   float operator[](size_t n) const { return values[n]; }

//   float x() const { return values[0]; }
//   float y() const { return values[1]; }
//   float z() const { return values[2]; }
//   float w() const { return values[3]; }
//   float r() const { return values[0]; }
//   float g() const { return values[1]; }
//   float b() const { return values[2]; }
//   float a() const { return values[3]; }

// private:
//   float values[N] = {};
// };

// template <auto N, class _type = decltype(N),
//           class = typename
//           std::enable_if<std::is_signed<_type>::value>::type>
// class mat {
// public:
//   mat() {}

//   template <class... T> mat(T... args) {
//     size_t i = 0;
//     for (const auto p : {args...}) {
//       values[i++] = p;
//     }
//   }

//   mat(_type arg) {
//     for (size_t i = 0; i < N; ++i) {
//       values[i + i * N] = arg;
//     }
//   }

//   vec<N, _type> operator[](size_t n) const {
//     vec<N, _type> v;
//     for (size_t i = 0; i < N; ++i) {
//       v[i] = values[n * N + i];
//     }
//     return v;
//   }

//   template <class T> mat &operator*(T arg) {
//     for (auto &i : values) {
//       i *= arg;
//     }

//     return *this;
//   }

//   vec<N, _type> operator*(const vec<N, _type> &arg) {
//     vec<N, _type> v;
//     for (size_t i = 0; i < N; ++i) {
//       v[i] = dot(*this[i], arg);
//     }
//     return v;
//   }

//   mat<N, _type> operator+(const mat<N, _type> &arg) {
//     mat<N, _type> m;
//     for (size_t i = 0; i < N; ++i) {
//       for (size_t j = 0; j < N; ++j) {
//         m[i][j] += arg[i][j];
//       }
//     }
//     return m;
//   }

// private:
//   float values[N] = {};
// };

// using vec3 = vec<3>;
// using vec4 = vec<4>;
// using mat3 = mat<3>;
// using mat4 = mat<4>;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
template <int N> using mat = glm::mat<N, N, glm::f32, glm::defaultp>;
using mat2 = glm::mat2;
using mat3 = glm::mat3;
using mat4 = glm::mat4;

mat4 rotate(const mat4 &m, const float degrees, const vec3 &axis);
mat4 scale(const mat4 &m, const vec3 &v);
mat4 translate(const mat4 &m, const vec3 &v);
vec3 normalize(const vec3 &v);
vec3 cross(const vec3 &a, const vec3 &b);
float dot(const vec3 &a, const vec3 &b);
float length(const vec3 &v);
mat3 transpose(const mat3 &m);
float radians(float angle);
mat4 inverse(const mat4 &m);
vec3 reflect(const vec3 &v, const vec3 &normal);

template <int N>
mat<N - 1> create_submatrix(const mat<N> &m, int skip_column, int skip_row) {
  mat<N - 1> result;
  int _row = 0, _col = 0;
  for (int i = 0; i < N * N; ++i) {
    int row = i % N, col = i / N;
    if (row == skip_row || col == skip_column) {
      continue;
    }

    _row = row;
    _col = col;
    if (row > skip_row) {
      --_row;
    }
    if (col > skip_column) {
      --_col;
    }

    result[_col][_row] = m[col][row];
  }

  return result;
}

template <int N> float determinant(const mat<N> &m) {
  float result = 0.f;
  for (int i = 0; i < N; ++i) {
    int sign = -i % 2;
    auto sub_m = create_submatrix(m, 0, i);
    result += sign * determinant(sub_m);
  }
  return result;
}

template <> float determinant(const mat<2> &m);

}; // namespace Transform
