#include <type_traits>
#define _USE_MATH_DEFINES
#include <array>
#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Transform {

// template <auto N, class _type = decltype(N),
//           class = typename std::enable_if<std::is_signed<_type>::value>::type>
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
//           class = typename std::enable_if<std::is_signed<_type>::value>::type>
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
using mat3 = glm::mat3;
using mat4 = glm::mat4;

void left(float degrees, vec3 &eye, vec3 &up);
void up(float degrees, vec3 &eye, vec3 &up);
mat4 lookAt(const vec3 &eye, const vec3 &center, const vec3 &up);
mat4 perspective(float fovy, float aspect, float zNear, float zFar);
mat3 rotate(const float degrees, const vec3 &axis);
mat4 scale(const float &sx, const float &sy, const float &sz);
mat4 translate(const float &tx, const float &ty, const float &tz);
vec3 upvector(const vec3 &up, const vec3 &zvec);

vec3 normalize(const vec3 &v);
vec3 cross(const vec3 &a, const vec3 &b);
float dot(const vec3 &a, const vec3 &b);
float length(const vec3 &v);
mat3 transpose(const mat3 &m);
float radians(float angle);
float determinant(const mat3& m);
mat3 inverse(const mat3 &m);
vec3 reflect(const vec3& v, const vec3& normal);
}; // namespace Transform
