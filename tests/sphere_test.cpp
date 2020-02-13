#include <test_common.h>
#include <intersections.h>


TEST(sphere_tests, intersection) {
  mat4 transform = translate(mat4(1.0f), vec3(0.0f, 0.0f, -1.0f));
  transform = rotate(transform, radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
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

  vec3 orig = s.inverted_transform * vec4(r.orig, 1.0f);
  vec3 dir = normalize(s.inverted_transform * vec4(r.dir, 0.0f));
  vec3 ip = orig + dir * dist;
  vec3 trans_p = s.transform * vec4(ip, 1.0f);

  ASSERT_FLOAT_EQ(1.4140722f, length(trans_p - r.orig));

  compare_vectors(trans_p, vec3(0.9999f, 0.0f, -0.9999f));

  vec3 normal = normalize(mat3(transpose(s.inverted_transform)) * vec3(intersection_point - s.pos));
  //vec3 normal = normalize(intersection_point - vec3(s.transform * vec4(s.pos, 1.0f)));
  compare_vectors(normal, vec3(1.0f, 0.0f, 0.0f));
}

TEST(sphere_tests, intersection2) {
  mat4 transform = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.5f));
  transform = rotate(transform, radians(45.0f), vec3(0.0f, 0.0f, 1.0f));
  transform = scale(transform, vec3(1.0f, 0.25f, 0.25f));

  Sphere s;
  s.radius = 1.0f;
  s.pos = vec3(0.0f, 0.0f, 0.0f);
  s.transform = transform;
  s.inverted_transform = inverse(transform);

  float dist = 0.0f;
  Ray r(vec3(.0f, -4.0f, 4.0f), normalize(vec3(-0.0788498148f, 0.724480212f, -0.684770823f)));
  bool result = intersection_sphere(s, r, dist);
  ASSERT_TRUE(result);
  // std::cerr << "dist: " << dist << "\n";

  // float dist2 = 0.0f;
  // vec3 orig000 = s.inverted_transform * vec4(r.orig, 1.0f);
  // vec3 dir000 = normalize(s.inverted_transform * vec4(r.dir, 0.0f));
  // vec3 ip = orig000 + dir000 * dist;
  // vec3 trans_p = s.transform * vec4(ip, 1.0f);
  // dist2 = length(trans_p - r.orig);
  // std::cerr << "dist2: " << dist2 << "\n";

  vec3 orig = s.inverted_transform * vec4(r.orig, 1.0f);
  vec3 dir = normalize(s.inverted_transform * vec4(r.dir, 0.0f));
  vec3 intersection_point = orig + dir * dist;
  vec3 normal = normalize(mat3(transpose(s.inverted_transform)) * vec3(intersection_point - s.pos));
  vec3 direction = normalize(vec3(1.f, 1.f, 3.f) - intersection_point);

  float n_dot_l = dot(normal, direction);
  ASSERT_GT(n_dot_l, 0.0f);
  vec4 lambert = vec4(1.0f, 0.f, 0.f, 1.0f) * std::max(n_dot_l, 0.0f);

  std::ostringstream ss;
  ss << "vector: " << std::setw(4) << "\n";
  ss << lambert.r << " " << lambert.g << " " << lambert.b << " " << lambert.a << "\n";
  std::cerr << ss.str();
}