#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <stack>
#include <string>
#include <thread>
#include <unordered_map>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <glm/glm.hpp>

#include <threadpool.h>
#include <transform.h>

//#define USE_GLM
#ifdef USE_GLM
#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
#include <glm/glm.hpp>
using namespace glm;
#else
using namespace Transform;
#endif

using Color = vec4;

struct Material {
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  vec4 emission;
  float shininess;
};

struct Sphere {
  float radius;
  vec3 pos;
  Material material;
  mat4 transform, inverted_transform;
};

struct Triangle {
  std::vector<vec3> vertices;
  vec3 normal;
  Material material;
};

struct TriangleNormals {
  std::vector<vec3> vertices;
  std::vector<vec3> normals;
  Material material;
};

struct DirectionLight {
  DirectionLight(const vec3 &dir, const Color &c) : dir(dir), color(c) {}
  vec3 dir;
  Color color;
};

struct PointLight {
  PointLight(const vec3 &pos, const Color &c, const vec3& attenuation) :
    pos(pos), color(c), attenuation(attenuation)
  {}

  vec3 pos;
  Color color;
  vec3 attenuation{ 1.0f, 0.0f, 0.0f };
};

enum ObjectType { TRIANGLE, TRIANGLE_NORMALS, SPHERE };

struct Object {
  ObjectType type;
  size_t index;
};

struct Settings {
  size_t width, height;
  float aspect;
  int depth = 5;
  std::string filename = "screenshot.png";
  vec3 eye_init;
  vec3 center;
  vec3 up_init;
  float fovy = 90;

  vec3 w, u, v;

  std::vector<Sphere> spheres;
  std::vector<Triangle> triangles;
  std::vector<TriangleNormals> triangle_normals;

  std::vector<DirectionLight> direct_lights;
  std::vector<PointLight> point_lights;

  std::vector<Object> objects;

  size_t threads_count = std::thread::hardware_concurrency() - 1;
};

Settings read_settings(const std::string &filename);

struct Ray {
  Ray(const vec3 &origin, const vec3 &direction)
      : orig(origin), dir(direction) {}
  vec3 orig;
  vec3 dir;
};

class Render {
public:
  Render(const Settings &s);
  ~Render();
  void update();
  void start_raytrace();

private:
  Color trace(const Ray &ray, int curr_depth = 0);
  void screeshot();
  void raytracer_process(size_t start, size_t end);
  void render_handling();
  bool cast_ray(const Ray &ray, vec3 &intersection_point, size_t &index);
  Color compute_shading(const vec3 &point, const vec3 &normal,
    size_t obj_index, const Material &m);
  vec4 compute_light(vec3 direction, vec4 lightcolor, vec3 normal, vec3 halfvec,
                     vec4 diffuse, vec4 specular, float shininess);

private:
  Settings s;

  sf::RenderWindow window;
  sf::Font font;
  sf::Text text{"", font, 15};

  sf::Texture texture;

  size_t pix_count;

  std::mutex guard;
  size_t progress = 0;

  size_t last_progress = 0;

  std::vector<uint8_t> draw_buffer;
  std::vector<uint8_t> buffer;

  std::thread task_provider;
  ThreadPool pool;

  std::chrono::time_point<std::chrono::system_clock> start_time;
};
