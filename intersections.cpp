#include <intersections.h>


bool intersection_sphere(const Sphere& s, const Ray& r, float& dist) {
  vec3 orig = s.inverted_transform * vec4(r.orig, 1.0f);
  vec3 dir = normalize(s.inverted_transform * vec4(r.dir, 0.0f));

  // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
  auto op = s.pos - orig;
  auto b = dot(op, dir);
  auto determinant = b * b - length(op)*length(op) + s.radius * s.radius;
  if (determinant < 0)
    return false;

  determinant = sqrt(determinant);
  auto minus_t = b - determinant;
  auto plus_t = b + determinant;
  if (minus_t < 0.000000001 && plus_t < 0.000000001)
    return false;

  dist = minus_t > 0.000000001 ? minus_t : plus_t;
  return true;
}

bool intersection_triangle_BROKEN(const Triangle& tri, const Ray& r, float& dist) {
  const vec3& v1 = tri.vertices[0];
  const vec3& v2 = tri.vertices[1];
  const vec3& v3 = tri.vertices[2];

  // Step 1: finding intersection_point
  float nDotRayDirection = dot(tri.normal, r.dir);
  if (fabs(nDotRayDirection) < std::numeric_limits<float>::epsilon())
    return false; // they are parallel, so they don't intersect

  float d = dot(tri.normal, v1);
  float t = -(dot(tri.normal, r.orig) + d) / nDotRayDirection;
  if (t < 0)
    return false; // the triangle is behind

  vec3 intersection_point = r.orig + t * r.dir;

  // Step 2: inside-outside test
  vec3 edge1 = v2 - v1;
  vec3 vp1 = intersection_point - v1;
  vec3 C = cross(edge1, vp1);
  if (dot(tri.normal, C) < 0)
    return false; // P is on the right side

  vec3 edge2 = v3 - v2;
  vec3 vp2 = intersection_point - v2;
  C = cross(edge2, vp2);
  if (dot(tri.normal, C) < 0)
    return false;

  vec3 edge3 = v1 - v3;
  vec3 vp3 = intersection_point - v3;
  C = cross(edge3, vp3);
  if (dot(tri.normal, C) < 0)
    return false;

  dist = t;
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

  float f = 1/a;
  vec3 s = r.orig - v1;
  float u = f * (dot(s, h));

  if (u < 0.0 || u > 1.0) {
    return false;
  }

  vec3 q = cross(s, e1);
  float v = f * dot(r.dir, q);

  if (v < 0.0 || u + v > 1.0) {
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
  const vec3& v1 = tri.vertices[0];
  const vec3& v2 = tri.vertices[1];
  const vec3& v3 = tri.vertices[2];

  vec3 N = cross(v2 - v1, v3 - v1);

  // Step 1: finding intersection_point
  float nDotRayDirection = dot(N, r.dir);
  if (fabs(nDotRayDirection) < std::numeric_limits<float>::epsilon())
    return false; // they are parallel, so they don't intersect

  float d = dot(N, v1);
  float t = -(dot(N, r.orig) + d) / nDotRayDirection;
  if (t < 0)
    return false; // the triangle is behind

  vec3 intersection_point = r.orig + t * r.dir;

  // Step 2: inside-outside test
  vec3 edge1 = v2 - v1;
  vec3 vp1 = intersection_point - v1;
  vec3 C = cross(edge1, vp1);
  if (dot(N, C) < 0)
    return false; // P is on the right side

  vec3 edge2 = v3 - v2;
  vec3 vp2 = intersection_point - v2;
  C = cross(edge2, vp2);
  if (dot(N, C) < 0)
    return false;

  vec3 edge3 = v1 - v3;
  vec3 vp3 = intersection_point - v3;
  C = cross(edge3, vp3);
  if (dot(N, C) < 0)
    return false;

  dist = t;
  return true;
}
