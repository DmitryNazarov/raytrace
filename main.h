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

#define _USE_MATH_DEFINES
#include <math.h>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>


#include <glm/glm.hpp>

#include <threadpool.h>
#include <transform.h>

using Color = glm::vec4;

struct Material {
  glm::vec4 ambient;
  glm::vec4 diffuse;
  glm::vec4 specular;
  glm::vec4 emission;
  float shininess;
};

struct Sphere {
  float radius;
  glm::vec3 pos;
  Material material;
  glm::mat4 transform, inverted_transform;
};

struct Triangle {
  std::vector<glm::vec3> vertices;
  glm::vec3 normal;
  Material material;
};

struct TriangleNormals {
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec3> normals;
  Material material;
};

struct DirectionLight {
  DirectionLight(const glm::vec3 &dir, const Color &c) : dir(dir), color(c) {}
  glm::vec3 dir;
  Color color;
};

struct PointLight {
  PointLight(const glm::vec3 &pos, const Color &c) : pos(pos), color(c) {}
  glm::vec3 pos;
  Color color;
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
  glm::vec3 eye_init;
  glm::vec3 center;
  glm::vec3 up_init;
  float fovy = 90;

  glm::vec3 w, u, v;

  std::vector<Sphere> spheres;
  std::vector<Triangle> triangles;
  std::vector<TriangleNormals> triangle_normals;

  std::vector<DirectionLight> direct_lights;
  std::vector<PointLight> point_lights;

  std::vector<Object> objects;

  float attenuation[3] = {1.0f, .0f, .0f};

  size_t threads_count = std::thread::hardware_concurrency() - 1;
};

Settings read_settings(const std::string &filename);

struct Ray {
  Ray(const glm::vec3 &origin, const glm::vec3 &direction)
      : orig(origin), dir(direction) {}
  glm::vec3 orig;
  glm::vec3 dir;
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
  bool cast_ray(const Ray &ray, glm::vec3 &intersection_point, size_t &index);
  Color compute_shading(const glm::vec3 &point, const glm::vec3 &normal,
                        const Material &m);
  glm::vec4 compute_light(glm::vec3 direction, glm::vec4 lightcolor,
                          glm::vec3 normal, glm::vec3 halfvec,
                          glm::vec4 diffuse, glm::vec4 specular,
                          float shininess);

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