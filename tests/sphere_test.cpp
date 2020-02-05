#include <test_common.h>
#include <intersections.h>


TEST(sphere_tests, intersection) {
  mat4 transform = translate(mat4(1.0f), vec3(0.0f, 0.0f, -1.0f));
  transform = rotate(transform, 90.0f, vec3(0.0f, 0.0f, 1.0f));
  transform = scale(transform, vec3(1.0f, 100.0f, 1.0f));

  Sphere s;
  s.radius = 0.01f;
  s.pos = vec3(0.0f, 0.0f, 0.0f);
  s.transform = transform;
  s.inverted_transform = inverse(transform);

  float dist = 0.0f;
  Ray r(vec3(.0f, .0f, .0f), normalize(vec3(1.0f, 0.0f, -1.0f)));
  bool result = intersection_sphere(s, r, dist);
  ASSERT_TRUE(result);

  vec3 intersection_point = r.orig + r.dir * dist;
  ASSERT_FLOAT_EQ(1.4140722f, length(intersection_point - r.orig));

  compare_vectors(intersection_point, vec3(0.9999f, 0.0f, -0.9999f));

  vec3 normal = normalize(intersection_point - vec3(s.transform * vec4(s.pos, 1.0f)));
  compare_vectors(normal, vec3(1.0f, 0.0f, 0.0001000266f));
}