#include <gtest/gtest.h>
#include <intersections.h>

using namespace Transform;


TEST(sphere_tests, intersection) {
  mat4 transform = translate(mat4(1.0f), vec3(0.0f, 0.0f, -1.0f));
  //transform = rotate(transform, 90.0f, vec3(0.0f, 0.0f, 1.0f));
  transform = scale(transform, vec3(1.0f, 2.0f, 1.0f));

  Sphere s;
  s.radius = 0.5f;
  s.pos = vec3(0.0f, 0.0f, 0.0f);
  s.transform = transform;
  s.inverted_transform = inverse(transform);

  float dist = 0.0f;
  Ray r(vec3(.0f, .0f, .0f), normalize(vec3(0.0f, 1.0f, -1.0f)));
  bool result = intersection_sphere(s, r, dist);

  vec3 intersection_point = r.orig + r.dir * dist;

  ASSERT_TRUE(result);
  ASSERT_FLOAT_EQ(0.84852844f, length(intersection_point - r.orig));
}