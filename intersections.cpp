#include <intersections.h>


bool intersection_sphere(const Sphere& s, const Ray& r, float& dist) {
  vec3 orig = s.inverted_transform * vec4(r.orig, 1.0f);
  vec3 dir = normalize(s.inverted_transform * vec4(r.dir, 0.0f));

  // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
  vec3 op = s.pos - orig;
  float b = dot(op, dir);
  float determinant = b * b - length(op) * length(op) + s.radius * s.radius;
  if (determinant < 0)
    return false;

  determinant = sqrt(determinant);
  float minus_t = b - determinant;
  float plus_t = b + determinant;
  if (minus_t < std::numeric_limits<float>::epsilon() &&
    plus_t < std::numeric_limits<float>::epsilon())
  {
    return false;
  }

  dist = minus_t > std::numeric_limits<float>::epsilon() ? minus_t : plus_t;
  return true;
}

bool intersection_triangle(const Triangle& tri, const Ray& r, float& dist) {
  const vec3& v1 = tri.vertices[0];
  const vec3& v2 = tri.vertices[1];
  const vec3& v3 = tri.vertices[2];

  vec3 e1 = v2 - v1;
  vec3 e2 = v3 - v1;
  vec3 h = cross(r.dir, e2);
  float a = dot(e1, h);

  if (abs(a) < std::numeric_limits<float>::epsilon()) {
    return false;
  }

  float f = 1.0f/a;
  vec3 s = r.orig - v1;
  float u = f * (dot(s, h));

  if (u < 0.0f || u > 1.0f) {
    return false;
  }

  vec3 q = cross(s, e1);
  float v = f * dot(r.dir, q);

  if (v < 0.0f || u + v > 1.0f) {
    return false;
  }

  float t = f * dot(e2, q);

  if (t > std::numeric_limits<float>::epsilon()) {
    dist = t / length(r.dir);
    return true;
  }
  else {
    return false;
  }
}

bool intersection_triangle(const TriangleNormals& tri, const Ray& r,
  float& dist) {
  Triangle t;
  t.vertices = tri.vertices;
  return intersection_triangle(t, r, dist);
}
