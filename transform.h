#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Transform {

void left(float degrees, glm::vec3 &eye, glm::vec3 &up);
void up(float degrees, glm::vec3 &eye, glm::vec3 &up);
glm::mat4 lookAt(const glm::vec3 &eye, const glm::vec3 &center,
                        const glm::vec3 &up);
glm::mat4 perspective(float fovy, float aspect, float zNear, float zFar);
glm::mat3 rotate(const float degrees, const glm::vec3 &axis);
glm::mat4 scale(const float &sx, const float &sy, const float &sz);
glm::mat4 translate(const float &tx, const float &ty, const float &tz);
glm::vec3 upvector(const glm::vec3 &up, const glm::vec3 &zvec);

}; // namespace Transform
