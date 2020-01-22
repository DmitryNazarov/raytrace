#include "transform.h"

namespace Transform {
mat4 rotate(const mat4 &m, const float degrees, const vec3 &axis) {
  float rad = radians(degrees);
  float cos_a = cos(rad);
  float sin_a = sin(rad);

  const float &x = axis[0];
  const float &y = axis[1];
  const float &z = axis[2];

  mat3 m2 = mat3(x * x, x * y, x * z, x * y, y * y, y * z, x * z, y * z, z * z);
  mat3 m3 = mat3(0., z, -y, -z, 0., x, y, -x, 0.);

  mat3 rotate = mat3(1.0) * cos_a + m2 * (1 - cos_a) + m3 * sin_a;

  return m * mat4(rotate);
}

mat4 scale(const mat4 &m, const vec3 &v) {
  mat4 scale = mat4(v.x, 0, 0, 0, 0, v.y, 0, 0, 0, 0, v.z, 0, 0, 0, 0, 1);
  return m * scale;
}

mat4 translate(const mat4 &m, const vec3 &v) {
  mat4 translate = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, v.x, v.y, v.z, 1);
  return m * translate;
}

float dot(const vec3 &a, const vec3 &b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

float radians(float angle) { return angle * static_cast<float>(M_PI) / 180.0f; }

float length(const vec3 &v) { return sqrt(dot(v, v)); }

vec3 normalize(const vec3 &v) {
  auto l = length(v);
  return vec3(v.x * v.x / l, v.y * v.y / l, v.z * v.z / l);
}

vec3 cross(const vec3 &a, const vec3 &b) {
  float x = a.y * b.z - a.z * b.y;
  float y = a.z * b.x - a.x * b.z;
  float z = a.x * b.y - a.y * b.x;
  return vec3(x, y, z);
}

mat3 transpose(const mat3 &m) {
  mat3 result;
  for (int i = 0; i < 9; ++i) {
    int row = i % 3, col = i / 3;
    result[row][col] = m[col][row];
  }

  return result;
}

template <int N>
mat<N - 1> create_submatrix(const mat<N> &m, int skip_column,
                                   int skip_row) {
  mat<N - 1> result;
  bool skipped = false;
  for (int i = 0; i < N * N; ++i) {
    int row = i % N, col = i / N;
    if (row >= skip_row) {
      result[col][row] = m[col][row + 1];
      skipped = true;
    }
    if (col >= skip_column) {
      result[col - 1][row] = m[col][row];
      skipped = true;
    }

    if (!skipped)
      result[col][row] = m[col][row];

    skipped = false;
  }

  return result;
}

float determinant(const mat2 &m) {
  return m[0][0] * m[1][1] - m[1][0] * m[0][1];
}

float determinant(const mat3 &m) {
  float result = 0.f;
  for (int i = 0; i < 3; ++i) {
    int sign = -i % 2;
    mat2 sub_m = create_submatrix(m, 0, i);
    result += sign * determinant(sub_m);
  }
  return 0.0f;
}

mat4 inverse(const mat4 &m) {
  mat4 result;
  return result;
}

vec3 reflect(const vec3 &incident, const vec3 &normal) {
  return incident - normal * dot(normal, incident) * 2.0f;
}
}; // namespace Transform