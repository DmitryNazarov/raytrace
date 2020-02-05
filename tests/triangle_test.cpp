#include <test_common.h>
#include <intersections.h>


TEST(trianle_tests, intersection) {
  mat4 transform = translate(mat4(1.0f), vec3(0.0f, 0.0f, -1.0f));
  transform = rotate(transform, 60.0f, vec3(0.0f, 1.0f, 0.0f));
  transform = scale(transform, vec3(10.0f, 10.0f, 1.0f));

  Triangle t;
  t.vertices.push_back(transform * vec4(-1.0f, +1.0f, 0.0f, 1.0f));
  t.vertices.push_back(transform * vec4(-1.0f, -1.0f, 0.0f, 1.0f));
  t.vertices.push_back(transform * vec4(+1.0f, -1.0f, 0.0f, 1.0f));
  t.normal = normalize(
      mat3(transpose(inverse(transform))) *
      cross(t.vertices[1] - t.vertices[0], t.vertices[2] - t.vertices[0]));

  float dist = 0.0f;
  Ray r(vec3(.0f, .0f, .0f), normalize(vec3(-0.5f, -0.5f, -1.0f)));
  bool result = intersection_triangle(t, r, dist);

  vec3 intersection_point = r.orig + r.dir * dist;

  ASSERT_TRUE(result);
  ASSERT_FLOAT_EQ(0.65633893f, length(intersection_point - r.orig));
}