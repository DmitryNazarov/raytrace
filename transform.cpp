#include "transform.h"

namespace Transform {
glm::mat3 rotate(const float degrees, const glm::vec3 &axis) {
  glm::mat3 result;
  float rad = glm::radians(degrees);
  float cos_a = cos(rad);
  float sin_a = sin(rad);

  const float &x = axis[0];
  const float &y = axis[1];
  const float &z = axis[2];

  glm::mat3 m2 =
      glm::mat3(x * x, x * y, x * z, x * y, y * y, y * z, x * z, y * z, z * z);
  glm::mat3 m3 = glm::mat3(0., z, -y, -z, 0., x, y, -x, 0.);

  result = glm::mat3(1.0) * cos_a + m2 * (1 - cos_a) + m3 * sin_a;

  return result;
}

void left(float degrees, glm::vec3 &eye, glm::vec3 &up) {
  glm::vec3 w = glm::normalize(eye);
  glm::vec3 u = glm::normalize(glm::cross(up, w));
  glm::vec3 v = glm::cross(w, u);

  eye = rotate(degrees, v) * eye;
  up = rotate(degrees, v) * up;
}

void up(float degrees, glm::vec3 &eye, glm::vec3 &up) {
  glm::vec3 w = glm::normalize(eye);
  glm::vec3 u = -glm::normalize(glm::cross(up, w));

  eye = rotate(degrees, u) * eye;
  up = rotate(degrees, u) * up;
}

glm::mat4 lookAt(const glm::vec3 &eye, const glm::vec3 &center,
                            const glm::vec3 &up) {
  glm::vec3 w = glm::normalize(eye - center);
  glm::vec3 u = glm::normalize(glm::cross(up, w));
  glm::vec3 v = glm::cross(w, u);

  glm::mat3 r = glm::mat3(u[0], v[0], w[0], u[1], v[1], w[1], u[2], v[2], w[2]);
  glm::vec3 r_t = r * (center - eye);

  glm::mat4 m = glm::mat4(u[0], v[0], w[0], 0, u[1], v[1], w[1], 0, u[2], v[2],
                          w[2], 0, r_t[0], r_t[1], r_t[2], 1);

  return m;
}

glm::mat4 perspective(float fovy, float aspect, float zNear,
                                 float zFar) {
  float theta = fovy / 2;
  float d = glm::cos(theta) / glm::sin(theta);

  float A = -(zFar + zNear) / (zFar - zNear);
  float B = -(2 * zFar * zNear) / (zFar - zNear);

  glm::mat4 ret =
      glm::mat4(d / aspect, 0, 0, 0, 0, d, 0, 0, 0, 0, A, -1, 0, 0, B, 0);

  return ret;
}

glm::mat4 scale(const float &sx, const float &sy, const float &sz) {
  glm::mat4 ret = glm::mat4(sx, 0, 0, 0, 0, sy, 0, 0, 0, 0, sz, 0, 0, 0, 0, 1);

  return ret;
}

glm::mat4 translate(const float &tx, const float &ty,
                               const float &tz) {
  glm::mat4 ret = glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, tx, ty, tz, 1);

  return ret;
}

glm::vec3 upvector(const glm::vec3 &up, const glm::vec3 &zvec) {
  glm::vec3 x = glm::cross(up, zvec);
  glm::vec3 y = glm::cross(zvec, x);
  glm::vec3 ret = glm::normalize(y);
  return ret;
}

}; // namespace Transform