#include <chrono>
#include <iostream>
#include <string>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <threadpool.h>
#include <intersections.h>
#include <settings_reader.h>

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
  void render_handling(const sf::Event &event);
  bool cast_ray(const Ray &ray, vec3 &intersection_point, int &index);
  Color compute_shading(const vec3 &point, const vec3& eye, const vec3 &normal,
    int obj_index, const Material &m);
  vec4 compute_light(const vec3& direction, const vec4& lightcolor, const vec3& normal,
    const vec3& halfvec, const vec4& diffuse, const vec4& specular, float shininess);
  void compensate_float_rounding_error(Ray& ray, const vec3& normal);

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

  bool show_debug = false;

  std::vector<uint8_t> draw_buffer;
  std::vector<uint8_t> buffer;

  std::thread task_provider;
  ThreadPool pool;

  std::chrono::time_point<std::chrono::system_clock> start_time;
};
