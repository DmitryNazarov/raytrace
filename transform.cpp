#include "transform.h"

namespace Transform {
mat3 rotate(const float degrees, const vec3 &axis) {
  mat3 result;
  float rad = radians(degrees);
  float cos_a = cos(rad);
  float sin_a = sin(rad);

  const float &x = axis[0];
  const float &y = axis[1];
  const float &z = axis[2];

  mat3 m2 = mat3(x * x, x * y, x * z, x * y, y * y, y * z, x * z, y * z, z * z);
  mat3 m3 = mat3(0., z, -y, -z, 0., x, y, -x, 0.);

  result = mat3(1.0) * cos_a + m2 * (1 - cos_a) + m3 * sin_a;

  return result;
}

mat4 scale(const float &sx, const float &sy, const float &sz) {
  mat4 ret = mat4(sx, 0, 0, 0, 0, sy, 0, 0, 0, 0, sz, 0, 0, 0, 0, 1);

  return ret;
}

mat4 translate(const float &tx, const float &ty, const float &tz) {
  mat4 ret = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, tx, ty, tz, 1);

  return ret;
}

float dot(const vec3 &a, const vec3 &b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

float radians(float angle) { return angle * M_PI / 180; }

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

mat3 transpose(const mat3 &m) { return mat3(); }

float determinant(const mat3 &m) { return 0.0f; }

mat3 inverse(const mat3 &m) {
  return m;
}

vec3 reflect(const vec3 &incident, const vec3 &normal) {
  return incident - normal * dot(normal, incident) * 2.0f;
}

}; // namespace Transform