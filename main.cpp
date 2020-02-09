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

void Render::compensate_float_rounding_error(Ray &ray, const vec3 &normal) {
  if (dot(ray.dir, normal) < 0.0f)
    ray.orig -= 1e-4f * normal;
  else
    ray.orig += 1e-4f * normal;
}

vec4 Render::compute_light(const vec3& direction, const vec4& lightcolor, const vec3& normal,
  const vec3& halfvec, const vec4& diffuse, const vec4& specular, float shininess)
{
  float n_dot_l = dot(normal, direction);
  vec4 lambert = diffuse * std::max(n_dot_l, 0.0f);

  float n_dot_h = dot(normal, halfvec);
  vec4 phong = specular * pow(std::max(n_dot_h, 0.0f), shininess);

  return lightcolor * (lambert + phong);
}

Color Render::compute_shading(const vec3 &point, const vec3 &normal,
                              int obj_index, const Material &m) {
  vec4 finalcolor = {0.0f, 0.0f, 0.0f, 1.0f};

  vec3 direction, half;
  vec3 eyedirn = normalize(s.eye_init - point);

  for (auto &i : s.direct_lights) {
    Ray shadow_ray(point, normalize(i.dir));
    compensate_float_rounding_error(shadow_ray, normal);

    vec3 hit_point;
    int index = 0;
    if (!cast_ray(shadow_ray, hit_point, index, obj_index)) {
      direction = normalize(-i.dir);
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
    int index = 0;
    bool is_hidden_by_other_obj = false;
    if (cast_ray(shadow_ray, hit_point, index, obj_index)) {
      auto l = length(hit_point - point);
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
  if (finalcolor.a > 1.0f)
    return vec4(vec3(finalcolor), 1.0f);
  return finalcolor;
}

bool Render::cast_ray(const Ray &ray, vec3 &intersection_point, int &index, int ignore_obj_index) {
  bool is_intersection = false;
  float dist = std::numeric_limits<float>::max(),
        d = std::numeric_limits<float>::max();

  for (size_t i = 0; i < s.objects.size(); ++i) {
    bool is_intersc = false;
    switch (s.objects[i].type) {
    case SPHERE: {
      is_intersc = intersection_sphere(s.spheres[s.objects[i].index], ray, d);
      break;
    }
    case TRIANGLE: {
      is_intersc = intersection_triangle(s.triangles[s.objects[i].index], ray, d);
      break;
    }
    case TRIANGLE_NORMALS: {
      is_intersc = intersection_triangle(s.triangle_normals[s.objects[i].index], ray, d);
      break;
    }
    default:
      std::cout << "Unknown object type!" << std::endl;
    }

    if (is_intersc) {
      if (d < dist && (i != ignore_obj_index || ignore_obj_index == -1)) {
        dist = d;
        index = static_cast<int>(i);
        is_intersection = true;
      }
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
  Color result{0.0f, 0.0f, 0.0f, 1.0f};

  if (++curr_depth == s.depth)
    return result;

  vec3 intersection_point;
  vec3 normal;
  vec4 specular;

  int i = 0;
  if (!cast_ray(ray, intersection_point, i))
    return result;

  Object &hit_obj = s.objects[i];
  switch (hit_obj.type) {
  case SPHERE: {
    Sphere &hit_sphere = s.spheres[hit_obj.index];
    specular = hit_sphere.material.specular;
    normal = normalize(intersection_point - vec3(hit_sphere.transform * vec4(hit_sphere.pos, 1.0f)));
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
  //result += specular * trace(secondary_ray, curr_depth);
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
    //size_t i = 385 + 212 * s.width;
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
      } else if (event.type == sf::Event::KeyPressed &&
                 event.key.code == sf::Keyboard::F2) {
        show_debug = !show_debug;
      }
    }
    window.clear();
    render_handling(event);
    window.display();
  }
}

void Render::render_handling(const sf::Event &event) {
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

  if (show_debug) {
    text.setString(std::to_string(event.mouseMove.x) + " " + std::to_string(event.mouseMove.y));
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
    //Render r(read_settings("E:\\Programming\\edx_cse167\\homework_hw3\\raytrace\\testscenes\\test_directional.test"));

    //Render r(read_settings("E:\\Programming\\edx_cse167\\homework_hw3\\raytrace\\hw3-submissionscenes\\scene7.test"));

    Render r(read_settings("E:\\Programming\\edx_cse167\\homework_hw3\\raytrace\\hw3-submissionscenes\\scene6.test"));

    //Render r(read_settings("E:\\Programming\\edx_cse167\\homework_hw3\\raytrace\\hw3-submissionscenes\\scene5.test"));

    //Render r(read_settings("E:\\Programming\\edx_cse167\\homework_hw3\\raytrace\\hw3-submissionscenes\\scene4-ambient.test"));
    //Render r(read_settings("E:\\Programming\\edx_cse167\\homework_hw3\\raytrace\\hw3-submissionscenes\\scene4-diffuse.test"));
    //Render r(read_settings("E:\\Programming\\edx_cse167\\homework_hw3\\raytrace\\hw3-submissionscenes\\scene4-emission.test"));
    //Render r(read_settings("E:\\Programming\\edx_cse167\\homework_hw3\\raytrace\\hw3-submissionscenes\\scene4-specular.test"));

    //Render r(read_settings("/home/dev/Work/github/raytrace/hw3-submissionscenes/scene6.test"));
    r.update();
  } catch (std::exception &e) {
    std::cout << "Error:" << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}