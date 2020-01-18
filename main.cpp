#include "main.h"


void debugMAT(const glm::mat4 &t)
{
  std::cout << "T:" << std::setw(4) << t[0][0] << " " 
  << std::setw(4) << t[1][0] << " " 
  << std::setw(4) << t[2][0] << " " 
  << std::setw(4) << t[3][0]<< std::endl;
  std::cout << "T:" << std::setw(4) << t[0][1] << " " 
  << std::setw(4) << t[1][1] << " " 
  << std::setw(4) << t[2][1] << " " 
  << std::setw(4) << t[3][1]<< std::endl;
  std::cout << "T:" << std::setw(4) << t[0][2] << " "
  << std::setw(4) << t[1][2] << " "
  << std::setw(4) << t[2][2] << " "
  << std::setw(4) << t[3][2]<< std::endl;
  std::cout << "T:" << std::setw(4) << t[0][3] << " "
  << std::setw(4) << t[1][3] << " "
  << std::setw(4) << t[2][3] << " "
  << std::setw(4) << t[3][3]<< std::endl;
}

Render::Render(const Settings &s) : s(s)
{
  if (!font.loadFromFile("times.ttf"))
    throw std::runtime_error("can't load times.ttf");

  window.create(sf::VideoMode(static_cast<unsigned>(s.width), static_cast<unsigned>(s.height)),
    "Ray tracer", sf::Style::Titlebar | sf::Style::Close);

  texture.create(static_cast<unsigned>(s.width), static_cast<unsigned>(s.height));

  draw_buffer_size = 4 * s.width * s.height;

  draw_buffer.resize(draw_buffer_size, 0);
  buffer.resize(draw_buffer_size, 0);
}

Render::~Render()
{
  raytracer_thread.join();
}

template<typename T>
bool readvals(std::stringstream& s, const int numvals, T* values)
{
  for (int i = 0; i < numvals; ++i)
  {
    s >> values[i];
    if (s.fail())
    {
      std::cout << "Failed reading value " << i << " will skip\n";
      return false;
    }
  }
  return true;
}

void rightmultiply(const glm::mat4& M, std::stack<glm::mat4>& transfstack)
{
  glm::mat4& T = transfstack.top();
  T = T * M;
}

Settings read_settings(const std::string& filename)
{
  Settings settings;

  std::vector<glm::vec3> vertices;
  std::vector<std::pair<glm::vec3, glm::vec3>> vertex_normals;

  Color ambient{ 0.2f, 0.2f, 0.2f, 1.0f };
  Color diffuse{ 0.0f, 0.0f, 0.0f, 0.0f };
  Color specular{ 0.0f, 0.0f, 0.0f, 0.0f };
  Color emission{ 0.0f, 0.0f, 0.0f, 0.0f };
  float shininess = .0;

  std::string str, cmd;
  std::ifstream in(filename);
  if (!in.is_open())
    return settings;

  std::stack<glm::mat4> transfstack;
  transfstack.emplace(1.0);  // identity

  while (getline(in, str))
  {
    if ((str.find_first_not_of(" \t\r\n") == std::string::npos) || (str[0] == '#'))
      continue;

    std::stringstream ss(str);
    ss >> cmd;

    if (cmd == "size")
    {
      int values[2];
      if (readvals(ss, 2, values))
      {
        settings.width = values[0];
        settings.height = values[1];
      }
    }
    else if (cmd == "camera")
    {
      float values[10];
      if (readvals(ss, 10, values))
      {
        settings.eye_init = glm::vec3(values[0], values[1], values[2]);
        settings.center = glm::vec3(values[3], values[4], values[5]);
        settings.up_init = glm::vec3(values[6], values[7], values[8]);
        settings.fovy = values[9];
      }
    }
    else if (cmd == "maxdepth")
    {
      int value;
      if (readvals(ss, 1, &value))
      {
        settings.depth = value;
      }
    }
    else if (cmd == "output")
    {
      std::string value;
      if (readvals(ss, 1, &value))
      {
        settings.filename = value;
      }
    }
    else if (cmd == "sphere")
    {
      float values[4];
      if (readvals(ss, 4, values))
      {
        Sphere s;
        s.radius = values[3];
        s.pos = glm::vec3(values[0], values[1], values[2]);
        s.transform = transfstack.top();
        s.inverted_transform = glm::inverse(transfstack.top());
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
    }
    else if (cmd == "translate")
    {
      float values[3];
      if (readvals(ss, 3, values))
      {
        rightmultiply(Transform::translate(values[0], values[1], values[2]), transfstack);
      }
    }
    else if (cmd == "scale")
    {
      float values[3];
      if (readvals(ss, 3, values))
      {
        rightmultiply(Transform::scale(values[0], values[1], values[2]), transfstack);
      }
    }
    else if (cmd == "rotate")
    {
      float values[4];
      if (readvals(ss, 4, values))
      {
        glm::vec3 axis = glm::normalize(glm::vec3(values[0], values[1], values[2]));
        rightmultiply(glm::mat4(Transform::rotate(values[3], axis)), transfstack);
      }
    }
    else if (cmd == "pushTransform")
    {
      transfstack.push(transfstack.top());
    }
    else if (cmd == "popTransform")
    {
      if (transfstack.size() <= 1)
      {
        std::cerr << "Stack has no elements.  Cannot Pop\n";
      }
      else
      {
        transfstack.pop();
      }
    }
    else if (cmd == "vertex")
    {
      float values[3];
      if (readvals(ss, 3, values))
      {
        vertices.emplace_back(values[0], values[1], values[2]);
      }
    }
    else if (cmd == "vertexnormal")
    {
      float values[6];
      if (readvals(ss, 6, values))
      {
        auto vertex = glm::vec3(values[0], values[1], values[2]);
        auto vertex_normal = glm::vec3(values[3], values[4], values[5]);
        vertex_normals.emplace_back(vertex, vertex_normal);
      }
    }
    else if (cmd == "tri")
    {
      int values[3];
      if (readvals(ss, 3, values))
      {
        Triangle t;
        t.vertices.push_back(t.transform * glm::vec4(vertices[values[0]], 1.0f));
        t.vertices.push_back(t.transform * glm::vec4(vertices[values[1]], 1.0f));
        t.vertices.push_back(t.transform * glm::vec4(vertices[values[2]], 1.0f));
        // t.vertices.push_back(glm::vec3(vertices[values[0]]));
        // t.vertices.push_back(glm::vec3(vertices[values[1]]));
        // t.vertices.push_back(glm::vec3(vertices[values[2]]));
        t.transform = transfstack.top();

        t.normal = glm::normalize(glm::cross(t.vertices[1] - t.vertices[0], t.vertices[2] - t.vertices[0]));

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
    }
    else if (cmd == "trinormal")
    {
      int values[3];
      if (readvals(ss, 3, values))
      {
        TriangleNormals t;
        t.vertices.push_back(vertex_normals[values[0]].first);
        t.vertices.push_back(vertex_normals[values[1]].first);
        t.vertices.push_back(vertex_normals[values[2]].first);
        t.normals.push_back(vertex_normals[values[0]].second);
        t.normals.push_back(vertex_normals[values[1]].second);
        t.normals.push_back(vertex_normals[values[2]].second);
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
    }
    else if (cmd == "directional")
    {
      float values[6];
      if (readvals(ss, 6, values))
      {
        auto dir = glm::vec3(values[0], values[1], values[2]);
        auto c = Color(values[3], values[4], values[5], 1.0f);
        settings.direct_lights.emplace_back(dir, c);
      }
    }
    else if (cmd == "point")
    {
      float values[6];
      if (readvals(ss, 6, values))
      {
        auto pos = glm::vec3(values[0], values[1], values[2]);
        auto c = Color(values[3], values[4], values[5], 1.0f);
        settings.point_lights.emplace_back(pos, c);
      }
    }
    else if (cmd == "ambient")
    {
      float values[3];
      if (readvals(ss, 3, values))
      {
        ambient = Color(values[0], values[1], values[2], 1.0f);
      }
    }
    else if (cmd == "attenuation")
    {
      float values[3];
      if (readvals(ss, 3, values))
      {
        settings.attenuation[0] = values[0];
        settings.attenuation[1] = values[1];
        settings.attenuation[2] = values[2];
      }
    }
    else if (cmd == "diffuse")
    {
      float values[3];
      if (readvals(ss, 3, values))
      {
        diffuse = Color(values[0], values[1], values[2], 1.0f);
      }
    }
    else if (cmd == "specular")
    {
      float values[3];
      if (readvals(ss, 3, values))
      {
        specular = Color(values[0], values[1], values[2], 1.0f);
      }
    }
    else if (cmd == "emission")
    {
      float values[3];
      if (readvals(ss, 3, values))
      {
        emission = Color(values[0], values[1], values[2], 1.0f);
      }
    }
    else if (cmd == "shininess")
    {
      float value;
      if (readvals(ss, 1, &value))
      {
        shininess = value;
      }
    }
    else if (cmd == "maxverts" || cmd == "maxvertnorms")
    {
      continue;
    }
    else
    {
      std::cerr << "Unknown Command: " << cmd << " Skipping \n";
    }
  }

  return settings;
}

bool intersection_sphere(Sphere& s, const Ray& r, float& dist)
{
  glm::vec3 orig = s.inverted_transform * glm::vec4(r.orig, 1.0f);
  glm::vec3 dir = glm::normalize(s.inverted_transform * glm::vec4(r.dir, 0.0f));

  //solve t * t * (r.dir * r.dir) + 2 * t * r.dir * (r.orig - i.pos) + (r.orig - i.pos) * (r.orig - i.pos) - i.radius^2 = 0;
  float a = glm::dot(dir, dir);
  float b = 2 * glm::dot(dir, (orig - s.pos));
  float c = glm::dot((orig - s.pos), (orig - s.pos)) - s.radius * s.radius;
  float d = b * b - 4 * a * c;

  if (d > 0)
  {
    float sqrt_d = glm::sqrt(d);
    float t1 = (-b + sqrt_d) / 2 * a;
    float t2 = (-b - sqrt_d) / 2 * a;

    float t = .0f;

    if (t1 < 0 || t2 < 0)
    {
      t = std::max(t1, t2);
    }
    else
    {
      t = std::min(t1, t2);
    }

    glm::vec3 intersection_point = orig + t * dir;
    glm::vec3 trans_point = s.transform * glm::vec4(intersection_point, 1.0f);
    dist = glm::length(trans_point - r.orig);

    return true;
  }
  else if (d == 0)
  {
    float t = -b / 2 * a;

    glm::vec3 intersection_point = orig + t * dir;
    glm::vec3 trans_point = s.transform * glm::vec4(intersection_point, 1.0f);
    dist = glm::length(trans_point - r.orig);

    return true;
  }

  return false;
}

bool intersection_triangle(const Triangle& tri, const Ray &r, float& dist)
{
  const glm::vec3& v1 = tri.vertices[0];
  const glm::vec3& v2 = tri.vertices[1];
  const glm::vec3& v3 = tri.vertices[2];

  glm::vec3 N = glm::cross(v2 - v1, v3 - v1);

  // Step 1: finding intersection_point
  float nDotRayDirection = glm::dot(N, r.dir);
  if (fabs(nDotRayDirection) < std::numeric_limits<float>::epsilon())
    return false; // they are parallel, so they don't intersect

  float d = glm::dot(N, v1);
  float t = -(glm::dot(N, r.orig) + d) / nDotRayDirection;
  if (t < 0) return false; // the triangle is behind

  glm::vec3 intersection_point = r.orig + t * r.dir;

  // Step 2: inside-outside test
  glm::vec3 edge1 = v2 - v1;
  glm::vec3 vp1 = intersection_point - v1;
  glm::vec3 C = glm::cross(edge1, vp1);
  if (glm::dot(N, C) < 0) return false; // P is on the right side

  glm::vec3 edge2 = v3 - v2;
  glm::vec3 vp2 = intersection_point - v2;
  C = glm::cross(edge2, vp2);
  if (glm::dot(N, C) < 0) return false;

  glm::vec3 edge3 = v1 - v3;
  glm::vec3 vp3 = intersection_point - v3;
  C = glm::cross(edge3, vp3);
  if (glm::dot(N, C) < 0) return false;

  dist = t;
  return true;
}

bool intersection_triangle(const TriangleNormals& tri, const Ray &r, float& dist)
{
  const glm::vec3& v1 = tri.vertices[0];
  const glm::vec3& v2 = tri.vertices[1];
  const glm::vec3& v3 = tri.vertices[2];

  glm::vec3 N = glm::cross(v2 - v1, v3 - v1);

  // Step 1: finding intersection_point
  float nDotRayDirection = glm::dot(N, r.dir);
  if (fabs(nDotRayDirection) < std::numeric_limits<float>::epsilon())
    return false; // they are parallel, so they don't intersect

  float d = glm::dot(N, v1);
  float t = -(glm::dot(N, r.orig) + d) / nDotRayDirection;
  if (t < 0) return false; // the triangle is behind

  glm::vec3 intersection_point = r.orig + t * r.dir;

  // Step 2: inside-outside test
  glm::vec3 edge1 = v2 - v1;
  glm::vec3 vp1 = intersection_point - v1;
  glm::vec3 C = glm::cross(edge1, vp1);
  if (glm::dot(N, C) < 0) return false; // P is on the right side

  glm::vec3 edge2 = v3 - v2;
  glm::vec3 vp2 = intersection_point - v2;
  C = glm::cross(edge2, vp2);
  if (glm::dot(N, C) < 0) return false;

  glm::vec3 edge3 = v1 - v3;
  glm::vec3 vp3 = intersection_point - v3;
  C = glm::cross(edge3, vp3);
  if (glm::dot(N, C) < 0) return false;

  dist = t;
  return true;
}

void compensate_float_rounding_error(Ray& ray, const glm::vec3& normal)
{
  if (glm::dot(ray.dir, normal) < 0.0f)
    ray.orig -= 0.01f * normal;
  else
    ray.orig += 0.01f * normal;
}

glm::vec4 Render::compute_light(glm::vec3 direction, glm::vec4 lightcolor, glm::vec3 normal, glm::vec3 halfvec,
  glm::vec4 diffuse, glm::vec4 specular, float shininess)
{
  float n_dot_l = glm::dot(normal, direction);
  glm::vec4 lambert = diffuse * std::max(n_dot_l, 0.0f);

  float n_dot_h = glm::dot(normal, halfvec);
  glm::vec4 phong = specular * pow(std::max(n_dot_h, 0.0f), shininess);

  return lightcolor * (lambert + phong);
}

Color Render::compute_shading(const glm::vec3 &point, const glm::vec3 &normal, const Material& m)
{
  glm::vec4 finalcolor = { 0.0f, 0.0f, 0.0f, 0.0f };

  glm::vec3 direction, half;
  glm::vec3 eyedirn = glm::normalize(s.eye_init - point);

  for (auto& i : s.direct_lights)
  {
    Ray shadow_ray(point, -i.dir);
    compensate_float_rounding_error(shadow_ray, normal);

    glm::vec3 hit_point;
    size_t index;
    if (!cast_ray(shadow_ray, hit_point, index))
    {
      direction = glm::normalize(i.dir);
      half = glm::normalize(direction + eyedirn);
      finalcolor += compute_light(direction, i.color, normal, half, m.diffuse, m.specular, m.shininess);
    }
  }

  for (auto& i : s.point_lights)
  {
    glm::vec3 lightdir = i.pos - point;
    Ray shadow_ray(point, lightdir);
    compensate_float_rounding_error(shadow_ray, normal);

    float dist = glm::length(lightdir);
    glm::vec3 hit_point;
    size_t index;
    bool is_intersection = false;
    if (cast_ray(shadow_ray, hit_point, index))
    {
      is_intersection = dist > glm::length(i.pos - hit_point);
    }

    if (!is_intersection)
    {
      direction = glm::normalize(lightdir);
      half = glm::normalize(direction + eyedirn);
      Color color = compute_light(direction, i.color, normal, half, m.diffuse, m.specular, m.shininess);
      float a = s.attenuation[0] + s.attenuation[1] * dist + s.attenuation[2] * dist * dist;
      finalcolor += color / a;
    }
  }

  finalcolor += m.ambient + m.emission;
  return finalcolor;
}

bool Render::cast_ray(const Ray& ray, glm::vec3& intersection_point, size_t& index)
{
  bool is_intersection = false;
  float dist = std::numeric_limits<float>::max(),
    d = std::numeric_limits<float>::max();

  for (size_t i = 0; i < s.objects.size(); ++i)
  {
    switch (s.objects[i].type)
    {
    case SPHERE:
    {
      if (intersection_sphere(s.spheres[s.objects[i].index], ray, d))
      {
        if (d < dist)
        {
          dist = d;
          index = i;
          is_intersection = true;
        }
      }
      break;
    }
    case TRIANGLE:
    {
      Triangle& t = s.triangles[s.objects[i].index];
      if (intersection_triangle(t, ray, d))
      {
        if (d < dist)
        {
          dist = d;
          index = i;
          is_intersection = true;
        }
      }
      break;
    }
    case TRIANGLE_NORMALS:
    {
      TriangleNormals& t = s.triangle_normals[s.objects[i].index];
      if (intersection_triangle(t, ray, d))
      {
        if (d < dist)
        {
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

glm::vec3 interpolate_normal(const TriangleNormals& triangle, const glm::vec3& intersection_point)
{
  glm::vec3 v0v1 = triangle.vertices[1] - triangle.vertices[0];
  glm::vec3 v0v2 = triangle.vertices[2] - triangle.vertices[0];

  glm::vec3 normal = glm::cross(v0v1, v0v2);
  float denom = glm::dot(normal, normal);

  glm::vec3 edge1 = triangle.vertices[2] - triangle.vertices[1];
  glm::vec3 vp1 = intersection_point - triangle.vertices[1];
  glm::vec3 C1 = glm::cross(edge1, vp1);
  float u = glm::dot(normal, C1) / denom;

  glm::vec3 edge2 = triangle.vertices[0] - triangle.vertices[2];
  glm::vec3 vp2 = intersection_point - triangle.vertices[2];
  glm::vec3 C2 = glm::cross(edge2, vp2);
  float v = glm::dot(normal, C2) / denom;

  glm::vec3 result = u * triangle.normals[0] + v * triangle.normals[1] + (1 - u - v) * triangle.normals[2];

  return glm::normalize(result);
}

Color mix_color(const Color& self_color, const Color& refl_color, const Color& coeff)
{
  float r = self_color.r * (1.0f - coeff.r) + refl_color.r * coeff.r;
  float g = self_color.r * (1.0f - coeff.g) + refl_color.g * coeff.g;
  float b = self_color.r * (1.0f - coeff.b) + refl_color.b * coeff.b;
  return Color(r, g, b, self_color.a);
}

Color Render::trace(const Ray &ray, int curr_depth)
{
  Color result{ 0.0f, 0.0f, 0.0f, 0.0f };

  if (++curr_depth == s.depth)
    return result;
  
  glm::vec3 intersection_point;
  glm::vec3 normal;
  glm::vec4 specular;

  size_t i = 0;
  if (!cast_ray(ray, intersection_point, i))
    return result;

  Object &hit_obj = s.objects[i];
  switch (hit_obj.type)
  {
  case SPHERE:
  {
    Sphere& hit_sphere = s.spheres[hit_obj.index];
    specular = hit_sphere.material.specular;

    //normal = glm::normalize(intersection_point - hit_sphere.pos);
    //normal = glm::normalize(glm::mat3(glm::transpose(hit_sphere.inverted_transform)) * normal);

    glm::vec4 p = hit_sphere.transform * glm::vec4(intersection_point, 1.0f);
    normal = glm::normalize(glm::mat3(glm::transpose(hit_sphere.inverted_transform)) * (glm::vec3(p) - hit_sphere.pos));

    result = compute_shading(intersection_point, normal, hit_sphere.material);
    break;
  }
  case TRIANGLE:
  {
    Triangle& hit_triangle = s.triangles[hit_obj.index];
    specular = hit_triangle.material.specular;
    normal = hit_triangle.normal;
    result = compute_shading(intersection_point, normal, hit_triangle.material);
    break;
  }
  case TRIANGLE_NORMALS:
  {
    TriangleNormals& hit_triangle = s.triangle_normals[hit_obj.index];
    specular = hit_triangle.material.specular;
    normal = interpolate_normal(hit_triangle, intersection_point);
    result = compute_shading(intersection_point, normal, hit_triangle.material);
    break;
  }
  default:
    std::cout << "Unknown object type!" << std::endl;
    return result;
  }

  Ray secondary_ray(intersection_point, glm::reflect(ray.dir, normal));
  compensate_float_rounding_error(secondary_ray, normal);
  result += specular * trace(secondary_ray, curr_depth);
  //result = mix_color(result, trace(secondary_ray, curr_depth), specular);

  return result;
}

void Render::start_raytrace()
{
  raytracer_thread = std::thread(&Render::raytracer_process, this);
}

void Render::raytracer_process()
{
  float aspect = static_cast<float>(s.width) / static_cast<float>(s.height);
  float znear = 0.1f, zfar = 99.0f;

  //projection = glm::perspective(glm::radians(s.fovy), aspect, znear, zfar);
  //view = glm::lookAt(s.eye_init, s.center, s.up_init);
  //glm::mat4 projection = Transform::perspective(glm::radians(s.fovy), aspect, znear, zfar);
  //view = Transform::lookAt(s.eye_init, s.center, s.up_init);

  glm::vec3 w = glm::normalize(s.eye_init - s.center);
  glm::vec3 u = glm::normalize(glm::cross(s.up_init, w));
  glm::vec3 v = glm::cross(w, u);

  size_t last_y = s.height;
  for (size_t i = 0; i < draw_buffer_size; i += 4)
  {
    size_t p = i / 4;
    size_t x = p % s.width;
    size_t y = p / s.width;

    float tan_theta = glm::tan(glm::radians(s.fovy) / 2);
    float a = (2 * (x + 0.5f) / static_cast<float>(s.width) - 1) * tan_theta * aspect;
    float b = (1 - 2 * (y + 0.5f) / static_cast<float>(s.height)) * tan_theta;
    glm::vec3 dir = glm::normalize(a * u + b * v - w);

    Color c = trace(Ray(s.eye_init, dir));
    //dir = glm::vec3(a, b, -1);
    //glm::vec4 t = view * glm::vec4(dir, 1.0f);
    //dir = glm::normalize(t / t.w);
    //Color c = trace(Ray(glm::vec3(0), dir));
    draw_buffer[i] = static_cast<int>(std::min(c.r, 1.0f) * 255);
    draw_buffer[i + 1] = static_cast<int>(std::min(c.g, 1.0f) * 255);
    draw_buffer[i + 2] = static_cast<int>(std::min(c.b, 1.0f) * 255);
    draw_buffer[i + 3] = static_cast<int>(std::min(c.a, 1.0f) * 255);

    {
      std::scoped_lock<std::mutex> lock(guard);
      progress = static_cast<int>(std::round((static_cast<float>(y) / s.height * 100)));
    }
  }
}

void Render::update()
{
  while (window.isOpen())
  {
    sf::Event event;
    while (window.pollEvent(event))
    {
      if (event.type == sf::Event::Closed)
      {
        window.close();
      }
      else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F1)
      {
        screeshot();
      }
    }
    window.clear();
    render_handling();
    window.display();
  }
}

void Render::render_handling()
{
  sf::Sprite sprite(texture);

  int curr_progress = 0;
  {
    std::scoped_lock<std::mutex> lock(guard);
    curr_progress = progress;
  }

  if (curr_progress == 100)
  {
    buffer = draw_buffer;
    text.setString("Completed!");
  }
  else
  {
    text.setString(std::to_string(curr_progress) + "%");
  }

  texture.update(buffer.data());
  window.draw(sprite);
  window.draw(text);
}

void Render::screeshot()
{
  texture.update(window);
  texture.copyToImage().saveToFile("test.png");
}

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cout << "Enter path to scene file" << std::endl;
    return EXIT_FAILURE;
  }

  try
  {
    Render r(read_settings(argv[1]));
    //Render r(read_settings("E:\\Programming\\edx_cse167\\homework_hw3\\testscenes\\scene0.test"));
    r.start_raytrace();
    r.update();
  }
  catch (std::exception &e)
  {
    std::cout << "Error:" << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}