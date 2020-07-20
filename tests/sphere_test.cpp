#include <test_common.h>
#include <intersections.h>


TEST(sphere_tests, intersection) {
  mat4 transform = translate(mat4(1.0f), vec3(0.0f, 0.0f, -1.0f));
  transform = rotate(transform, radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
  transform = scale(transform, vec3(0.01f, 1.0f, 0.01f));

  Sphere s;
  s.radius = 1.0f;
  s.pos = vec3(0.0f, 0.0f, 0.0f);
  s.transform = transform;
  s.inverted_transform = inverse(transform);

  float dist = 0.0f;
  Ray r(vec3(.0f, .0f, 0.0f), normalize(vec3(0.1f, 0.0f, -1.0f)));
  bool result = intersection_sphere(s, r, dist);
  ASSERT_TRUE(result);

  vec3 orig = s.inverted_transform * vec4(r.orig, 1.0f);
  vec3 dir = normalize(s.inverted_transform * vec4(r.dir, 0.0f));
  vec3 ip = orig + dir * dist;
  vec3 trans_p = s.transform * vec4(ip, 1.0f);

  ASSERT_NEAR(0.99f, length(trans_p - r.orig), 0.01);

  compare_vectors(trans_p, vec3(0.099f, 0.0f, -0.99f));

  vec3 normal = normalize(mat3(transpose(s.inverted_transform)) * vec3(ip - s.pos));
  compare_vectors(normal, vec3(0.0f, 0.0f, 1.0f));
}
