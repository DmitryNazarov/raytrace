#include "main.h"


Render::Render(const Settings &s) : s(s) {
  if (!font.loadFromFile("times.ttf"))
    throw std::runtime_error("can't load times.ttf");

  window.create(sf::VideoMode(static_cast<unsigned>(s.width),
                              static_cast<unsigned>(s.height)),
                "Ray tracer", sf::Style::Titlebar | sf::Style::Close);

  if (!texture.create(static_cast<unsigned>(s.width),
                      static_cast<unsigned>(s.height)))
    throw std::runtime_error("texture creation fail");

  pix_count = s.width * s.height;

  draw_buffer.resize(pix_count * 4, 0);
  buffer.resize(pix_count * 4, 0);

  start_raytrace();
}

Render::~Render() { task_provider.join(); }

template <typename T>
bool readvals(std::stringstream &s, const int numvals, T *values) {
  for (int i = 0; i < numvals; ++i) {
    s >> values[i];
    if (s.fail()) {
      std::cout << "Failed reading value " << i << " will skip\n";
      return false;
    }
  }
  return true;
}

void rightmultiply(const mat4 &M, std::stack<mat4> &transfstack) {
  mat4 &T = transfstack.top();
  T = T * M;
}

Settings read_settings(const std::string &filename) {
  Settings settings;

  std::vector<vec3> vertices;
  std::vector<std::pair<vec3, vec3>> vertex_normals;

  Color ambient{0.2f, 0.2f, 0.2f, 1.0f};
  Color diffuse{0.0f, 0.0f, 0.0f, 0.0f};
  Color specular{0.0f, 0.0f, 0.0f, 0.0f};
  Color emission{0.0f, 0.0f, 0.0f, 0.0f};
  float shininess = .0;
  vec3 attenuation{ 1.0f, 0.0f, 0.0f };

  std::string str, cmd;
  std::ifstream in(filename);
  if (!in.is_open())
    return settings;

  std::stack<mat4> transfstack;
  transfstack.push(mat4(1.0)); // identity

  while (getline(in, str)) {
    if ((str.find_first_not_of(" \t\r\n") == std::string::npos) ||
        (str[0] == '#'))
      continue;

    std::stringstream ss(str);
    ss >> cmd;

    if (cmd == "size") {
      int values[2];
      if (readvals(ss, 2, values)) {
        settings.width = values[0];
        settings.height = values[1];
        settings.aspect = static_cast<float>(settings.width) /
                          static_cast<float>(settings.height);
      }
    } else if (cmd == "camera") {
      float values[10];
      if (readvals(ss, 10, values)) {
        settings.eye_init = vec3(values[0], values[1], values[2]);
        settings.center = vec3(values[3], values[4], values[5]);
        settings.up_init = vec3(values[6], values[7], values[8]);
        settings.fovy = values[9];

        settings.w = normalize(settings.eye_init - settings.center);
        settings.u = normalize(cross(settings.up_init, settings.w));
        settings.v = cross(settings.w, settings.u);
      }
    } else if (cmd == "maxdepth") {
      int value;
      if (readvals(ss, 1, &value)) {
        settings.depth = value;
      }
    } else if (cmd == "output") {
      std::string value;
      if (readvals(ss, 1, &value)) {
        settings.filename = value;
      }
    } else if (cmd == "sphere") {
      float values[4];
      if (readvals(ss, 4, values)) {
        Sphere s;
        s.radius = values[3];
        s.pos = vec3(values[0], values[1], values[2]);
        s.transform = transfstack.top();
        s.inverted_transform = inverse(transfstack.top());
        s.material.ambient = ambient;
        s.material.diffuse = diffuse;
        s.material.specular = specular;
        s.material.emission = emission;
        s.material.shininess = shininess;
        settings.spheres.push_back(s);
        Object o;
        o.type = SPHERE;
        o.index = settings.spheres.size() - 1;
        settings.objects.push_back(o);
      }
    } else if (cmd == "translate") {
      float values[3];
      if (readvals(ss, 3, values)) {
        // rightmultiply(translate(values[0], values[1], values[2]),
        //              transfstack);
        rightmultiply(
            translate(mat4(1.0f), vec3(values[0], values[1], values[2])),
            transfstack);
      }
    } else if (cmd == "scale") {
      float values[3];
      if (readvals(ss, 3, values)) {
        // rightmultiply(scale(values[0], values[1], values[2]),
        //               transfstack);
        rightmultiply(scale(mat4(1.0f), vec3(values[0], values[1], values[2])),
                      transfstack);
      }
    } else if (cmd == "rotate") {
      float values[4];
      if (readvals(ss, 4, values)) {
        vec3 axis = normalize(vec3(values[0], values[1], values[2]));
        // rightmultiply(mat4(rotate(values[3], axis)),
        //               transfstack);
        rightmultiply(rotate(mat4(1.0f), values[3], axis), transfstack);
      }
    } else if (cmd == "pushTransform") {
      transfstack.push(transfstack.top());
    } else if (cmd == "popTransform") {
      if (transfstack.size() <= 1) {
        std::cerr << "Stack has no elements.  Cannot Pop\n";
      } else {
        transfstack.pop();
      }
    } else if (cmd == "vertex") {
      float values[3];
      if (readvals(ss, 3, values)) {
        vertices.emplace_back(values[0], values[1], values[2]);
      }
    } else if (cmd == "vertexnormal") {
      float values[6];
      if (readvals(ss, 6, values)) {
        auto vertex = vec3(values[0], values[1], values[2]);
        auto vertex_normal = vec3(values[3], values[4], values[5]);
        vertex_normals.emplace_back(vertex, vertex_normal);
      }
    } else if (cmd == "tri") {
      int values[3];
      if (readvals(ss, 3, values)) {
        Triangle t;
        t.vertices.push_back(transfstack.top() *
                             vec4(vertices[values[0]], 1.0f));
        t.vertices.push_back(transfstack.top() *
                             vec4(vertices[values[1]], 1.0f));
        t.vertices.push_back(transfstack.top() *
                             vec4(vertices[values[2]], 1.0f));
        t.normal = normalize(cross(t.vertices[2] - t.vertices[0],
          t.vertices[1] - t.vertices[0]));
        t.material.ambient = ambient;
        t.material.diffuse = diffuse;
        t.material.specular = specular;
        t.material.emission = emission;
        t.material.shininess = shininess;
        settings.triangles.push_back(t);
        Object o;
        o.type = TRIANGLE;
        o.index = settings.triangles.size() - 1;
        settings.objects.push_back(o);
      }
    } else if (cmd == "trinormal") {
      int values[3];
      if (readvals(ss, 3, values)) {
        TriangleNormals t;
        t.vertices.push_back(transfstack.top() *
                             vec4(vertex_normals[values[0]].first, 1.0f));
        t.vertices.push_back(transfstack.top() *
                             vec4(vertex_normals[values[1]].first, 1.0f));
        t.vertices.push_back(transfstack.top() *
                             vec4(vertex_normals[values[2]].first, 1.0f));
        t.vertices.push_back(transfstack.top() *
                             vec4(vertex_normals[values[0]].second, 1.0f));
        t.vertices.push_back(transfstack.top() *
                             vec4(vertex_normals[values[1]].second, 1.0f));
        t.vertices.push_back(transfstack.top() *
                             vec4(vertex_normals[values[2]].second, 1.0f));
        t.material.ambient = ambient;
        t.material.diffuse = diffuse;
        t.material.specular = specular;
        t.material.emission = emission;
        t.material.shininess = shininess;
        settings.triangle_normals.push_back(t);
        Object o;
        o.type = TRIANGLE_NORMALS;
        o.index = settings.triangle_normals.size() - 1;
        settings.objects.push_back(o);
      }
    } else if (cmd == "directional") {
      float values[6];
      if (readvals(ss, 6, values)) {
        auto dir = vec3(values[0], values[1], values[2]);
        auto c = Color(values[3], values[4], values[5], 1.0f);
        settings.direct_lights.emplace_back(dir, c);
      }
    } else if (cmd == "point") {
      float values[6];
      if (readvals(ss, 6, values)) {
        auto pos = vec3(values[0], values[1], values[2]);
        auto c = Color(values[3], values[4], values[5], 1.0f);
        settings.point_lights.emplace_back(pos, c, attenuation);
      }
    } else if (cmd == "ambient") {
      float values[3];
      if (readvals(ss, 3, values)) {
        ambient = Color(values[0], values[1], values[2], 1.0f);
      }
    } else if (cmd == "attenuation") {
      float values[3];
      if (readvals(ss, 3, values)) {
        attenuation[0] = values[0];
        attenuation[1] = values[1];
        attenuation[2] = values[2];
      }
    } else if (cmd == "diffuse") {
      float values[3];
      if (readvals(ss, 3, values)) {
        diffuse = Color(values[0], values[1], values[2], 1.0f);
      }
    } else if (cmd == "specular") {
      float values[3];
      if (readvals(ss, 3, values)) {
        specular = Color(values[0], values[1], values[2], 1.0f);
      }
    } else if (cmd == "emission") {
      float values[3];
      if (readvals(ss, 3, values)) {
        emission = Color(values[0], values[1], values[2], 1.0f);
      }
    } else if (cmd == "shininess") {
      float value;
      if (readvals(ss, 1, &value)) {
        shininess = value;
      }
    } else if (cmd == "maxverts" || cmd == "maxvertnorms") {
      continue;
    } else {
      std::cerr << "Unknown Command: " << cmd << " Skipping \n";
    }
  }

  return settings;
}

bool intersection_sphere(Sphere &s, const Ray &r, float &dist) {
  vec3 orig = s.inverted_transform * vec4(r.orig, 1.0f);
  vec3 dir = normalize(s.inverted_transform * vec4(r.dir, 0.0f));

  // solve t * t * (r.dir * r.dir) + 2 * t * r.dir * (r.orig - i.pos) + (r.orig
  // - i.pos) * (r.orig - i.pos) - i.radius^2 = 0;
  float a = dot(dir, dir);
  float b = 2 * dot(dir, (orig - s.pos));
  float c = dot((orig - s.pos), (orig - s.pos)) - s.radius * s.radius;
  float d = b * b - 4 * a * c;

  if (d > 0) {
    float sqrt_d = sqrt(d);
    float t1 = (-b + sqrt_d) / 2 * a;
    float t2 = (-b - sqrt_d) / 2 * a;

    float t = .0f;

    if (t1 < 0 || t2 < 0) {
      t = std::max(t1, t2);
    } else {
      t = std::min(t1, t2);
    }

    vec3 intersection_point = orig + t * dir;
    vec3 trans_point = s.transform * vec4(intersection_point, 1.0f);
    dist = length(trans_point - r.orig);

    return true;
  } else if (d == 0) {
    float t = -b / 2 * a;

    vec3 intersection_point = orig + t * dir;
    vec3 trans_point = s.transform * vec4(intersection_point, 1.0f);
    dist = length(trans_point - r.orig);

    return true;
  }

  return false;
}

bool intersection_triangle(const Triangle &tri, const Ray &r, float &dist) {
  const vec3 &v1 = tri.vertices[0];
  const vec3 &v2 = tri.vertices[1];
  const vec3 &v3 = tri.vertices[2];

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

bool intersection_triangle(const TriangleNormals &tri, const Ray &r,
                           float &dist) {
  const vec3 &v1 = tri.vertices[0];
  const vec3 &v2 = tri.vertices[1];
  const vec3 &v3 = tri.vertices[2];

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

void compensate_float_rounding_error(Ray &ray, const vec3 &normal) {
  if (dot(ray.dir, normal) < 0.0f)
    ray.orig -= 0.0001f * normal;
  else
    ray.orig += 0.0001f * normal;
}

vec4 Render::compute_light(vec3 direction, vec4 lightcolor, vec3 normal,
                           vec3 halfvec, vec4 diffuse, vec4 specular,
                           float shininess) {
  float n_dot_l = dot(normal, direction);
  vec4 lambert = diffuse * std::max(n_dot_l, 0.0f);

  float n_dot_h = dot(normal, halfvec);
  vec4 phong = specular * pow(std::max(n_dot_h, 0.0f), shininess);

  return lightcolor * (lambert + phong);
}

Color Render::compute_shading(const vec3 &point, const vec3 &normal,
                              size_t obj_index, const Material &m) {
  vec4 finalcolor = {0.0f, 0.0f, 0.0f, 1.0f};

  vec3 direction, half;
  vec3 eyedirn = normalize(s.eye_init - point);

  for (auto &i : s.direct_lights) {
    Ray shadow_ray(point, normalize(-i.dir));
    compensate_float_rounding_error(shadow_ray, normal);

    vec3 hit_point;
    size_t index = 0;
    if (!cast_ray(shadow_ray, hit_point, index) && obj_index != index) {
      direction = normalize(i.dir);
      half = normalize(direction + eyedirn);
      finalcolor += compute_light(direction, i.color, normal, half, m.diffuse,
                                  m.specular, m.shininess);
    }
  }

  for (auto &i : s.point_lights) {
    vec3 lightdir = i.pos - point;
    direction = normalize(lightdir);
    Ray shadow_ray(point, direction);
    compensate_float_rounding_error(shadow_ray, normal);

    float dist = length(lightdir);
    vec3 hit_point;
    size_t index = 0;
    bool is_hidden_by_other_obj = false;
    if (cast_ray(shadow_ray, hit_point, index) && obj_index != index) {
      is_hidden_by_other_obj = length(hit_point - point) < dist;
    }

    if (!is_hidden_by_other_obj) {
      half = normalize(direction + eyedirn);
      Color color = compute_light(direction, i.color, normal, half, m.diffuse,
                                  m.specular, m.shininess);
      float a = i.attenuation[0] + i.attenuation[1] * dist +
                i.attenuation[2] * dist * dist;
      finalcolor += color / a;
    }
  }

  finalcolor += m.ambient + m.emission;
  if (finalcolor.a > 255)
    return vec4(vec3(finalcolor), 1.0f);
  return finalcolor;
}

bool Render::cast_ray(const Ray &ray, vec3 &intersection_point, size_t &index) {
  bool is_intersection = false;
  float dist = std::numeric_limits<float>::max(),
        d = std::numeric_limits<float>::max();

  for (size_t i = 0; i < s.objects.size(); ++i) {
    switch (s.objects[i].type) {
    case SPHERE: {
      if (intersection_sphere(s.spheres[s.objects[i].index], ray, d)) {
        if (d < dist) {
          dist = d;
          index = i;
          is_intersection = true;
        }
      }
      break;
    }
    case TRIANGLE: {
      Triangle &t = s.triangles[s.objects[i].index];
      if (intersection_triangle(t, ray, d)) {
        if (d < dist) {
          dist = d;
          index = i;
          is_intersection = true;
        }
      }
      break;
    }
    case TRIANGLE_NORMALS: {
      TriangleNormals &t = s.triangle_normals[s.objects[i].index];
      if (intersection_triangle(t, ray, d)) {
        if (d < dist) {
          dist = d;
          index = i;
          is_intersection = true;
        }
      }
      break;
    }
    default:
      std::cout << "Unknown object type!" << std::endl;
    }
  }

  intersection_point = ray.orig + dist * ray.dir;

  return is_intersection;
}

vec3 interpolate_normal(const TriangleNormals &triangle,
                        const vec3 &intersection_point) {
  vec3 v0v1 = triangle.vertices[1] - triangle.vertices[0];
  vec3 v0v2 = triangle.vertices[2] - triangle.vertices[0];

  vec3 normal = cross(v0v1, v0v2);
  float denom = dot(normal, normal);

  vec3 edge1 = triangle.vertices[2] - triangle.vertices[1];
  vec3 vp1 = intersection_point - triangle.vertices[1];
  vec3 C1 = cross(edge1, vp1);
  float u = dot(normal, C1) / denom;

  vec3 edge2 = triangle.vertices[0] - triangle.vertices[2];
  vec3 vp2 = intersection_point - triangle.vertices[2];
  vec3 C2 = cross(edge2, vp2);
  float v = dot(normal, C2) / denom;

  vec3 result = u * triangle.normals[0] + v * triangle.normals[1] +
                (1 - u - v) * triangle.normals[2];

  return normalize(result);
}

Color mix_color(const Color &self_color, const Color &refl_color,
                const Color &coeff) {
  float r = self_color.r * (1.0f - coeff.r) + refl_color.r * coeff.r;
  float g = self_color.g * (1.0f - coeff.g) + refl_color.g * coeff.g;
  float b = self_color.b * (1.0f - coeff.b) + refl_color.b * coeff.b;
  return Color(r, g, b, self_color.a);
}

Color Render::trace(const Ray &ray, int curr_depth) {
  Color result{0.0f, 0.0f, 0.0f, 0.0f};

  if (++curr_depth == s.depth)
    return result;

  vec3 intersection_point;
  vec3 normal;
  vec4 specular;

  size_t i = 0;
  if (!cast_ray(ray, intersection_point, i))
    return result;

  Object &hit_obj = s.objects[i];
  switch (hit_obj.type) {
  case SPHERE: {
    Sphere &hit_sphere = s.spheres[hit_obj.index];
    specular = hit_sphere.material.specular;

    //vec3 p = hit_sphere.transform * vec4(intersection_point, 1.0f);
    /*normal = normalize(mat3(transpose(hit_sphere.inverted_transform)) *
      (vec3(p) - hit_sphere.pos));*/

    vec3 p = hit_sphere.inverted_transform * vec4(intersection_point, 1.0f);
    //normal = normalize(hit_sphere.transform * vec4(normalize(vec3(p) - hit_sphere.pos), 0.f));

    //normal = normalize(mat3(hit_sphere.inverted_transform) * (p - hit_sphere.pos));
    //normal = normalize(mat3(hit_sphere.transform) * (p - hit_sphere.pos));
    normal = normalize((p - hit_sphere.pos));

    result = compute_shading(intersection_point, normal, i, hit_sphere.material);
    break;
  }
  case TRIANGLE: {
    Triangle &hit_triangle = s.triangles[hit_obj.index];
    specular = hit_triangle.material.specular;
    normal = hit_triangle.normal;
    result = compute_shading(intersection_point, normal, i, hit_triangle.material);
    break;
  }
  case TRIANGLE_NORMALS: {
    TriangleNormals &hit_triangle = s.triangle_normals[hit_obj.index];
    specular = hit_triangle.material.specular;
    normal = interpolate_normal(hit_triangle, intersection_point);
    result = compute_shading(intersection_point, normal, i, hit_triangle.material);
    break;
  }
  default:
    std::cout << "Unknown object type!" << std::endl;
    return result;
  }

  Ray secondary_ray(intersection_point, reflect(ray.dir, normal));
  compensate_float_rounding_error(secondary_ray, normal);
  result += specular * trace(secondary_ray, curr_depth);
  //result = mix_color(result, trace(secondary_ray, curr_depth), specular);

  return result;
}

void Render::start_raytrace() {
  start_time = std::chrono::system_clock::now();
  task_provider = std::thread([this] {
    size_t step = s.width;
    for (size_t i = 0; i < pix_count; i += step) {
      pool.add_work([this, i, step]() { raytracer_process(i, i + step); });
    }
  });
}

void Render::raytracer_process(size_t start, size_t end) {
  for (size_t i = start; i < end; ++i) {
    size_t x = i % s.width;
    size_t y = i / s.width;

    float tan_theta = tan(radians(s.fovy) / 2);
    float a = (2 * (x + 0.5f) / static_cast<float>(s.width) - 1) * tan_theta *
              s.aspect;
    float b = (1 - 2 * (y + 0.5f) / static_cast<float>(s.height)) * tan_theta;
    vec3 dir = normalize(a * s.u + b * s.v - s.w);

    Color c = trace(Ray(s.eye_init, dir));

    draw_buffer[4 * i] = static_cast<int>(std::min(c.r, 1.0f) * 255);
    draw_buffer[4 * i + 1] = static_cast<int>(std::min(c.g, 1.0f) * 255);
    draw_buffer[4 * i + 2] = static_cast<int>(std::min(c.b, 1.0f) * 255);
    draw_buffer[4 * i + 3] = static_cast<int>(std::min(c.a, 1.0f) * 255);

    {
      std::scoped_lock<std::mutex> lock(guard);
      ++progress;
    }
  }
}

void Render::update() {
  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        window.close();
      } else if (event.type == sf::Event::KeyPressed &&
                 event.key.code == sf::Keyboard::F1) {
        screeshot();
      }
    }
    window.clear();
    render_handling();
    window.display();
  }
}

void Render::render_handling() {
  sf::Sprite sprite(texture);

  if (last_progress != pix_count) {
    size_t curr_progress = 0;
    {
      std::scoped_lock<std::mutex> lock(guard);
      curr_progress = progress;
    }

    if (curr_progress == pix_count) {
      buffer = draw_buffer;
      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now() - start_time);
      text.setString(""); //text.setString("Completed! " + std::to_string(elapsed.count()) + " ms");
    } else {
      int p = static_cast<int>(std::round(static_cast<float>(curr_progress) /
                                          static_cast<float>(pix_count) * 100));
      text.setString(std::to_string(p) + "%");
    }

    last_progress = curr_progress;
  }

  texture.update(buffer.data());
  window.draw(sprite);
  window.draw(text);
}

void Render::screeshot() {
  texture.update(window);
  texture.copyToImage().saveToFile(s.filename);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "Enter path to scene file" << std::endl;
    // return EXIT_FAILURE;
  }

  try {
    // Render r(read_settings(argv[1]));
    //Render r(read_settings("E:\\Programming\\edx_cse167\\homework_hw3\\raytrace\\testscenes\\scene_test.test"));

    //Render r(read_settings("E:\\Programming\\edx_cse167\\homework_hw3\\raytrace\\hw3-submissionscenes\\scene7.test"));

    //Render r(read_settings("E:\\Programming\\edx_cse167\\homework_hw3\\raytrace\\hw3-submissionscenes\\scene6.test"));

    //Render r(read_settings("E:\\Programming\\edx_cse167\\homework_hw3\\raytrace\\hw3-submissionscenes\\scene5.test"));

    //Render r(read_settings("E:\\Programming\\edx_cse167\\homework_"
    //  "hw3\\raytrace\\hw3-submissionscenes\\scene4-ambient.test"));
    //Render r(read_settings("E:\\Programming\\edx_cse167\\homework_hw3\\raytrace\\hw3-submissionscenes\\scene4-diffuse.test"));
    //Render r(read_settings("E:\\Programming\\edx_cse167\\homework_"
    //  "hw3\\raytrace\\hw3-submissionscenes\\scene4-emission.test"));
    //Render r(read_settings("E:\\Programming\\edx_cse167\\homework_"
    //  "hw3\\raytrace\\hw3-submissionscenes\\scene4-specular.test"));

    Render r(read_settings("/home/dev/Work/github/raytrace/hw3-submissionscenes/scene6.test"));
    r.update();
  } catch (std::exception &e) {
    std::cout << "Error:" << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}