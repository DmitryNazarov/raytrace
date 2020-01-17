#include <iostream>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <thread>
#include <fstream>
#include <mutex>
#include <chrono>
#include <stack>
#include <sstream>
#define _USE_MATH_DEFINES
#include <math.h>

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

#include <glm/glm.hpp>

#include <transform.h>


using Color = glm::vec4;

struct Material
{
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

struct Triangle
{
  std::vector<glm::vec3> vertices;
  glm::vec3 normal;
  Material material;
  glm::mat4 transform;
};

struct TriangleNormals
{
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec3> normals;
  Material material;
  glm::mat4 transform;
};

struct DirectionLight
{
  DirectionLight(const glm::vec3 &dir, const Color &c)
    : dir(dir), color(c)
  {}
  glm::vec3 dir;
  Color color;
};

struct PointLight
{
  PointLight(const glm::vec3& pos, const Color& c)
    : pos(pos), color(c)
  {}
  glm::vec3 pos;
  Color color;
};

enum ObjectType
{
  TRIANGLE,
  TRIANGLE_NORMALS,
  SPHERE
};

struct Object
{
  ObjectType type;
  size_t index;
};

struct Settings
{
  size_t width = 180, height = 120;
  int depth = 5;
  std::string filename = "screenshot.png";
  glm::vec3 eye_init;
  glm::vec3 center;
  glm::vec3 up_init;
  float fovy = 90;

  std::vector<Sphere> spheres;
  std::vector<Triangle> triangles;
  std::vector<TriangleNormals> triangle_normals;

  std::vector<DirectionLight> direct_lights;
  std::vector<PointLight> point_lights;

  std::vector<Object> objects;

  float attenuation[3] = {1.0f, .0f, .0f};
};

Settings read_settings(const std::string& filename);

struct Ray
{
  Ray(const glm::vec3& origin, const glm::vec3& direction) : orig(origin), dir(direction)
  {}
  glm::vec3 orig;
  glm::vec3 dir;
};

class Render
{
public:
  Render(const Settings &s);
  ~Render();
  void update();
  void start_raytrace();

private:
  Color trace(const Ray& ray, int curr_depth = 0);
  void screeshot();
  void raytracer_process();
  void render_handling();
  bool cast_ray(const Ray& ray, glm::vec3& intersection_point, size_t& index);
  Color compute_shading(const glm::vec3& point, const glm::vec3& normal, const Material& m);
  glm::vec4 compute_light(glm::vec3 direction, glm::vec4 lightcolor, glm::vec3 normal, glm::vec3 halfvec,
    glm::vec4 diffuse, glm::vec4 specular, float shininess);

private:
  Settings s;

  sf::RenderWindow window;
  sf::Font font;
  sf::Text text{"", font, 15};

  sf::Texture texture;

  size_t draw_buffer_size;
  std::mutex guard;
  int progress = 0;

  std::vector<uint8_t> draw_buffer;
  std::vector<uint8_t> buffer;

  std::thread raytracer_thread;
};