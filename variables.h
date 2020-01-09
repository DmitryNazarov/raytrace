/*****************************************************************************/
/* This is the program skeleton for homework 2 in CSE167 by Ravi Ramamoorthi */
/* Extends HW 1 to deal with shading, more transforms and multiple objects   */
/*****************************************************************************/

// This is the basic include file for the global variables in the program.  
// Since all files need access to it, we define EXTERN as either blank or 
// extern, depending on if included in the main program or not.  

#include <stdint.h>

#include <glm/glm.hpp>

EXTERN int amount; // The amount of rotation for each arrow press
EXTERN glm::vec3 eye; // The (regularly updated) vector coordinates of the eye 
EXTERN glm::vec3 up;  // The (regularly updated) vector coordinates of the up 

EXTERN glm::vec3 eyeinit;
EXTERN glm::vec3 upinit;
EXTERN glm::vec3 center;
EXTERN int amountinit;
EXTERN int w, h;
EXTERN float fovy;

EXTERN bool useGlu; // Toggle use of "official" opengl/glm transform vs user 
EXTERN uint32_t vertexshader, fragmentshader, shaderprogram; // shaders
EXTERN glm::mat4 projection, modelview; // The mvp matrices
EXTERN uint32_t projectionPos, modelviewPos; // Uniform locations of the above matrices
static enum { view, translate, scale } transop; // which operation to transform 
enum shape { cube, sphere, teapot };
EXTERN float sx, sy; // the scale in x and y 
EXTERN float tx, ty; // the translation in x and y

// Lighting parameter array, similar to that in the fragment shader
const int numLights = 10;
EXTERN float lightposn[4 * numLights]; // Light Positions
EXTERN float lightcolor[4 * numLights]; // Light Colors
EXTERN float lightransf[4 * numLights]; // Lights transformed by modelview
EXTERN int numused;                     // How many lights are used 

// Materials (read from file) 
// With multiple objects, these are colors for each.
EXTERN float ambient[4];
EXTERN float diffuse[4];
EXTERN float specular[4];
EXTERN float emission[4];
EXTERN float shininess;

// For multiple objects, read from a file.  
const int maxobjects = 10;
EXTERN int numobjects;
EXTERN struct object {
  shape type;
  float size;
  float ambient[4];
  float diffuse[4];
  float specular[4];
  float emission[4];
  float shininess;
  glm::mat4 transform;
} objects[maxobjects];

// Variables to set uniform params for lighting fragment shader 
EXTERN uint32_t lightcol;
EXTERN uint32_t lightpos;
EXTERN uint32_t numusedcol;
EXTERN uint32_t enablelighting;
EXTERN uint32_t ambientcol;
EXTERN uint32_t diffusecol;
EXTERN uint32_t specularcol;
EXTERN uint32_t emissioncol;
EXTERN uint32_t shininesscol;
