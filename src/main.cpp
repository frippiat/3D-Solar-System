// ----------------------------------------------------------------------------
// main.cpp
//
//  Created on: 24 Jul 2020
//      Author: Kiwon Um
//        Mail: kiwon.um@telecom-paris.fr
//
// Description: IGR201 Practical; OpenGL and Shaders (DO NOT distribute!)
//
// Copyright 2020-2024 Kiwon Um
//
// The copyright to the computer program(s) herein is the property of Kiwon Um,
// Telecom Paris, France. The program(s) may be used and/or copied only with
// the written permission of Kiwon Um or in accordance with the terms and
// conditions stipulated in the agreement/contract under which the program(s)
// have been supplied.
// ----------------------------------------------------------------------------

#define _USE_MATH_DEFINES

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Mesh.hpp"
#include "Camera.hpp"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>
#include <thread>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// constants
const static float kSizeSun = 1;
const static float kSizeEarth = 0.5;
const static float kSizeMoon = 0.25;
const static float kRadOrbitEarth = 10;
const static float kRadOrbitMoon = 2;

// Window parameters
GLFWwindow *g_window = nullptr;

// GPU objects
GLuint g_program = 0; // A GPU program contains at least a vertex shader and a fragment shader

// OpenGL identifiers
GLuint g_vao = 0;
GLuint g_posVbo = 0;
GLuint g_ibo = 0;
GLuint g_colVbo = 0; //Color VBO

// All vertex positions packed in one array [x0, y0, z0, x1, y1, z1, ...]
std::vector<float> g_vertexPositions;
// All triangle indices packed in one array [v00, v01, v02, v10, v11, v12, ...] with vij the index of j-th vertex of the i-th triangle
std::vector<unsigned int> g_triangleIndices;
std::vector<float> g_vertexColors;

//Sphere mesh
std::shared_ptr<Mesh> sphere;
glm::mat4 modelMatrixSun, modelMatrixEarth, modelMatrixMoon, modelMatrixMars, modelMatrixVenus, modelMatrixUranus, modelMatrixSaturn, modelMatrixNeptune, modelMatrixJupiter, modelMatrixMercury;

GLuint g_earthTexID, g_moonTexID, g_marsTexID, g_venusTexID, g_uranusTexID, g_saturnTexID, g_neptuneTexID, g_jupiterTexID, g_mercuryTexID;




GLuint loadTextureFromFileToGPU(const std::string &filename) {
  // Loading the image in CPU memory using stb_image
  int width, height, numComponents;
  unsigned char *data = stbi_load(filename.c_str(), &width, &height, &numComponents, 0);
  GLuint texID; // OpenGL texture identifier
  glGenTextures(1, &texID); // generate an OpenGL texture container
  glBindTexture(GL_TEXTURE_2D, texID); // activate the texture
  // Setup the texture filtering option and repeat mode; check www.opengl.org for details.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // Fill the GPU texture with the data stored in the CPU image
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
  // Free useless CPU memory
  stbi_image_free(data);
  glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture
  return texID;
}

// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window.
void windowSizeCallback(GLFWwindow* window, int width, int height) {
  g_camera.setAspectRatio(static_cast<float>(width)/static_cast<float>(height));
  glViewport(0, 0, (GLint)width, (GLint)height); // Dimension of the rendering region in the window
}

// Executed each time a key is entered.
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if(action == GLFW_PRESS && key == GLFW_KEY_W) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else if(action == GLFW_PRESS && key == GLFW_KEY_F) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  } else if(action == GLFW_PRESS && (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)) {
    glfwSetWindowShouldClose(window, true); // Closes the application if the escape key is pressed
  } 
  //Move camera down on Z-axis
  else if((action == GLFW_PRESS || action == GLFW_REPEAT) && key == GLFW_KEY_X) {
    const glm::vec3 camPosition = g_camera.getPosition();
    g_camera.setPosition(glm::vec3(camPosition[0], camPosition[1]-1.0, camPosition[2]));
  } 
  // Move camera up on Z-axis
  else if((action == GLFW_PRESS || action == GLFW_REPEAT) && key == GLFW_KEY_S) {
    const glm::vec3 camPosition = g_camera.getPosition();
    g_camera.setPosition(glm::vec3(camPosition[0], camPosition[1]+1.0, camPosition[2]));
  } 
  // Move camera up on Y-axis 
  else if((action == GLFW_PRESS || action == GLFW_REPEAT) && key == GLFW_KEY_UP) {
    const glm::vec3 camPosition = g_camera.getPosition();
    g_camera.setPosition(glm::vec3(camPosition[0], camPosition[1], camPosition[2]-1.0));
  } 
  //  Move camera down on Y-axis
  else if((action == GLFW_PRESS || action == GLFW_REPEAT) && key == GLFW_KEY_DOWN) {
    const glm::vec3 camPosition = g_camera.getPosition();
    g_camera.setPosition(glm::vec3(camPosition[0], camPosition[1], camPosition[2]+1.0));
  } 
  // Move camera right on X-axis
  else if((action == GLFW_PRESS || action == GLFW_REPEAT) && key == GLFW_KEY_RIGHT) {
    const glm::vec3 camPosition = g_camera.getPosition();
    g_camera.setPosition(glm::vec3(camPosition[0]+1.0, camPosition[1], camPosition[2]));
  } 
  // Move camera left on X-axis
  else if((action == GLFW_PRESS || action == GLFW_REPEAT) && key == GLFW_KEY_LEFT) {
    const glm::vec3 camPosition = g_camera.getPosition();
    g_camera.setPosition(glm::vec3(camPosition[0]-1.0, camPosition[1], camPosition[2]));
  }
  // Return camera to initial position
  else if((action == GLFW_PRESS || action == GLFW_REPEAT) && key == GLFW_KEY_C) { //set back in starting position
    const glm::vec3 camPosition = g_camera.getPosition();
    g_camera.setPosition(glm::vec3(0.0, 0.0 , 30.0));
  }
}

void errorCallback(int error, const char *desc) {
  std::cout <<  "Error " << error << ": " << desc << std::endl;
}

void initGLFW() {
  glfwSetErrorCallback(errorCallback);

  // Initialize GLFW, the library responsible for window management
  if(!glfwInit()) {
    std::cerr << "ERROR: Failed to init GLFW" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  // Before creating the window, set some option flags
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

  g_window = glfwCreateWindow(
    1024,768,
    "Interactive 3D Applications (OpenGL) - Simple Solar System",
    nullptr, nullptr);
  if(!g_window) {
    std::cerr << "ERROR: Failed to open window" << std::endl;
    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  // Load the OpenGL context in the GLFW window using GLAD OpenGL wrangler
  glfwMakeContextCurrent(g_window);
  glfwSetWindowSizeCallback(g_window, windowSizeCallback);
  glfwSetKeyCallback(g_window, keyCallback);
}

void initOpenGL() {
  // Load extensions for modern OpenGL
  if(!gladLoadGL(glfwGetProcAddress)) {
    std::cerr << "ERROR: Failed to initialize OpenGL context" << std::endl;
    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  glCullFace(GL_BACK); // Specifies the faces to cull (here the ones pointing away from the camera)
  glEnable(GL_CULL_FACE); // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
  glDepthFunc(GL_LESS);   // Specify the depth test for the z-buffer
  glEnable(GL_DEPTH_TEST);      // Enable the z-buffer test in the rasterization
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // specify the background color, used any time the framebuffer is cleared
}

// Loads the content of an ASCII file in standard C++ string
std::string file2String(const std::string &filename) {
  std::ifstream t(filename.c_str());
  std::stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str(); 
}

// Loads and compile a shader, before attaching it to a program
void loadShader(GLuint program, GLenum type, const std::string &shaderFilename) {
  GLuint shader = glCreateShader(type); // Create the shader, e.g., a vertex shader to be applied to every single vertex of a mesh
  std::string shaderSourceString = file2String(shaderFilename); // Loads the shader source from a file to a C++ string
  const GLchar *shaderSource = (const GLchar *)shaderSourceString.c_str(); // Interface the C++ string through a C pointer
  glShaderSource(shader, 1, &shaderSource, NULL); // load the vertex shader code
  glCompileShader(shader);
  GLint success;
  GLchar infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if(!success) {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    std::cout << "ERROR in compiling " << shaderFilename << "\n\t" << infoLog << std::endl;
  }
  glAttachShader(program, shader);
  glDeleteShader(shader);
}


void initGPUprogram() {
  g_program = glCreateProgram(); // Create a GPU program, i.e., two central shaders of the graphics pipeline
  loadShader(g_program, GL_VERTEX_SHADER, "vertexShader.glsl");
  loadShader(g_program, GL_FRAGMENT_SHADER, "fragmentShader.glsl");
  glLinkProgram(g_program); // The main GPU program is ready to handle streams of polygons

  glUseProgram(g_program);

  // Load textures for Earth, Moon, and Mars
  g_earthTexID = loadTextureFromFileToGPU("media/earth.jpg");
  g_moonTexID = loadTextureFromFileToGPU("media/moon.jpg");
  g_marsTexID = loadTextureFromFileToGPU("media/mars.jpg");
  g_venusTexID = loadTextureFromFileToGPU("media/venus.jpg");
  g_uranusTexID = loadTextureFromFileToGPU("media/uranus.jpg");
  g_saturnTexID = loadTextureFromFileToGPU("media/saturn.jpg");
  g_neptuneTexID = loadTextureFromFileToGPU("media/neptune.jpg");
  g_jupiterTexID = loadTextureFromFileToGPU("media/jupiter.jpg");
  g_mercuryTexID = loadTextureFromFileToGPU("media/mercury.jpg");

  glUniform1i(glGetUniformLocation(g_program, "material.albedoTex"), 0); // texture unit 0
  
}


// Define your mesh(es) in the CPU memory
void initCPUgeometry() { //THIS FUNCTION IS NOT USED ANYMORE WHEN  WE START USING THE MESH CLASS
  // TODO: add vertices and indices for your mesh(es)
  
  g_vertexPositions = { // positions of each vertex
    0.f, 0.f, 0.f, //(x,y,z) position of vertex 1
    1.f, 0.f, 0.f, //(x,y,z) position of vertex 2
    0.f, 1.f, 0.f  //(x,y,z) position of vertex 3
  };
  g_triangleIndices = { 0, 1, 2 }; // indices just for one triangle
  
  g_vertexColors = { //assigns colors to each vertex: first line is the RGB-values of first vertex, second line is RGB values of second vertex etc.
    1.f, 1.f, 1.f, //(r,g,b)-values of vertex 0
    0.f, 1.f, 1.f, //(r,g,b)-values of vertex 1
    0.f, 1.f, 0.f  //(r,g,b)-values of vertex 2
    };

}

void initGPUgeometry() { //THIS FUNCTION IS NOT USED ANYMORE ONCE WE START USING THE MESH CLASS
  // Create a single handle, vertex array object that contains attributes,
  // vertex buffer objects (e.g., vertex's position, normal, and color)
#ifdef _MY_OPENGL_IS_33_
  glGenVertexArrays(1, &g_vao); // If your system doesn't support OpenGL 4.5, you should use this instead of glCreateVertexArrays.
#else
  glCreateVertexArrays(1, &g_vao);
#endif


  glBindVertexArray(g_vao);

  // Generate a GPU buffer to store the positions of the vertices
  size_t vertexBufferSize = sizeof(float)*g_vertexPositions.size(); // Gather the size of the buffer from the CPU-side vector
#ifdef _MY_OPENGL_IS_33_ //Irrelevant since Vincent's laptop has OpenGL version 4.6
  glGenBuffers(1, &g_posVbo);
  glBindBuffer(GL_ARRAY_BUFFER, g_posVbo);
  glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, g_vertexPositions.data(), GL_DYNAMIC_READ);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);
  glEnableVertexAttribArray(0);
  
  glGenBuffers(1, &g_colVbo);
  glBindBuffer(GL_ARRAY_BUFFER, g_colVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertexColors), g_vertexColors.data(), GL_DYNAMIC_READ);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0); // attention: index=1
  glEnableVertexAttribArray(1); 
#else
  //Transferring the positions of the vertices in a Vertex Buffer Object VBO (OpenGL object that stores vertex data)
  glCreateBuffers(1, &g_posVbo);
  glBindBuffer(GL_ARRAY_BUFFER, g_posVbo);
  glNamedBufferStorage(g_posVbo, vertexBufferSize, g_vertexPositions.data(), GL_DYNAMIC_STORAGE_BIT); // Create a data storage on the GPU and fill it from a CPU array
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);
  glEnableVertexAttribArray(0);

  glCreateBuffers(1, &g_colVbo);
  glBindBuffer(GL_ARRAY_BUFFER, g_colVbo);
  glNamedBufferStorage(g_colVbo, sizeof(g_vertexColors), g_vertexColors.data(), GL_DYNAMIC_STORAGE_BIT); // Create a data storage for colors
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0); // attention: index=1
  glEnableVertexAttribArray(1); // attention: index=1
#endif

  // Same for an index buffer object that stores the list of indices of the triangles forming the mesh
  size_t indexBufferSize = sizeof(unsigned int)*g_triangleIndices.size();
#ifdef _MY_OPENGL_IS_33_  //Irrelevant since Vincent's laptop has OpenGL version 4.6
  glGenBuffers(1, &g_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, g_triangleIndices.data(), GL_DYNAMIC_READ);
#else
  glCreateBuffers(1, &g_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo);
  glNamedBufferStorage(g_ibo, indexBufferSize, g_triangleIndices.data(), GL_DYNAMIC_STORAGE_BIT);
#endif

  glBindVertexArray(0); // deactivate the VAO for now, will be activated again when rendering
}

void initCamera() {
  int width, height;
  glfwGetWindowSize(g_window, &width, &height);
  g_camera.setAspectRatio(static_cast<float>(width)/static_cast<float>(height));

  g_camera.setPosition(glm::vec3(0.0, 0.0 , 30.0));
  g_camera.setNear(0.1);
  g_camera.setFar(80.1);
}

void init() {
  initGLFW();
  initOpenGL();
  
  /* TRIANGLE
  initCPUgeometry();
  */
  initGPUprogram();

  sphere = Mesh::genSphere(16); // Create a sphere mesh
  sphere->init(); // Initialize its GPU buffers
  
  /* TRIANGLE
  initGPUgeometry();
  */

 
  initCamera();
}

void clear() {
  glDeleteProgram(g_program);

  glfwDestroyWindow(g_window);
  glfwTerminate();
}

// The main rendering call
void render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  const glm::mat4 viewMatrix = g_camera.computeViewMatrix();
  const glm::mat4 projMatrix = g_camera.computeProjectionMatrix();
  const glm::vec3 camPosition = g_camera.getPosition();

  glUniformMatrix4fv(glGetUniformLocation(g_program, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
  glUniformMatrix4fv(glGetUniformLocation(g_program, "projMat"), 1, GL_FALSE, glm::value_ptr(projMatrix));
  glUniform3fv(glGetUniformLocation(g_program, "camPosition"), 1, glm::value_ptr(camPosition));

  // Sun rendering
  glUniform1i(glGetUniformLocation(g_program, "isSun"), 1);
  glUniform3f(glGetUniformLocation(g_program, "objectColor"), 1.0f, 1.0f, 0.0f);
  modelMatrixSun = glm::mat4(1.0f);
  modelMatrixSun = glm::scale(modelMatrixSun, glm::vec3(kSizeSun));
  glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrixSun));
  sphere->render();

  glActiveTexture(GL_TEXTURE0);
  glUniform1i(glGetUniformLocation(g_program, "isSun"), 0);

  // Earth rendering
  glBindTexture(GL_TEXTURE_2D, g_earthTexID);
  glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrixEarth));
  sphere->render();

  // Moon rendering
  glBindTexture(GL_TEXTURE_2D, g_moonTexID);
  glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrixMoon));
  sphere->render();

  // Mars rendering
  glBindTexture(GL_TEXTURE_2D, g_marsTexID);
  glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrixMars));
  sphere->render();

  // Venus rendering
  glBindTexture(GL_TEXTURE_2D, g_venusTexID);
  glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrixVenus));
  sphere->render();

  // Jupiter rendering
  glBindTexture(GL_TEXTURE_2D, g_jupiterTexID);
  glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrixJupiter));
  sphere->render();

  // Saturn rendering
  glBindTexture(GL_TEXTURE_2D, g_saturnTexID);
  glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrixSaturn));
  sphere->render();

  // Uranus rendering
  glBindTexture(GL_TEXTURE_2D, g_uranusTexID);
  glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrixUranus));
  sphere->render();

  // Neptune rendering
  glBindTexture(GL_TEXTURE_2D, g_neptuneTexID);
  glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrixNeptune));
  sphere->render();

  // Mercury rendering
  glBindTexture(GL_TEXTURE_2D, g_mercuryTexID);
  glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrixMercury));
  sphere->render();


  glBindTexture(GL_TEXTURE_2D, 0);
}


void update(const float currentTimeInSec) {
  // Constants for orbital and rotational periods
  float earthRotationSpeed = 1.0f;
  float earthOrbitSpeed = 0.5f*earthRotationSpeed;
  float moonRotationSpeed = 2.0f*earthRotationSpeed;
  float moonOrbitSpeed = 2.0f;

  modelMatrixEarth = glm::mat4(1.0f);
  glm::mat4 earthTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(kRadOrbitEarth * cos(0.3f * currentTimeInSec), 0.0f, kRadOrbitEarth * sin(0.3f * currentTimeInSec)));
  glm::mat4 earthScale = glm::scale(glm::mat4(1.0f), glm::vec3(kSizeEarth));
  glm::mat4 earthRotate = glm::rotate(glm::mat4(1.0f), 0.6f * currentTimeInSec, glm::vec3(sin(glm::radians(23.5)), cos(glm::radians(23.5)), 0.0f));
  modelMatrixEarth= earthTranslate * earthScale * earthRotate;



  modelMatrixMoon = glm::mat4(1.0f); 
  glm::mat4 moonTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(kRadOrbitMoon * cos(0.6 * currentTimeInSec), 0.0f, kRadOrbitMoon * sin(0.6 * currentTimeInSec)));
  glm::mat4 moonRotate = glm::rotate(glm::mat4(1.0f), 0.6f * currentTimeInSec, glm::vec3(0, 1, 0.0f));
  glm::mat4 moonScale = glm::scale(glm::mat4(1.0f), glm::vec3(kSizeMoon));
  modelMatrixMoon = earthTranslate * moonTranslate * moonRotate * moonScale;

  // Mars parameters
  modelMatrixMars = glm::mat4(1.0f);
  float marsOrbitAngle = 0.3f * currentTimeInSec;
  modelMatrixMars = glm::translate(modelMatrixMars, glm::vec3(glm::cos(marsOrbitAngle) * 15.0f, 0.0f, glm::sin(marsOrbitAngle) * 15.0f));
  modelMatrixMars = glm::rotate(modelMatrixMars, 0.8f * currentTimeInSec, glm::vec3(0.0f, 1.0f, 0.0f));
  modelMatrixMars = glm::scale(modelMatrixMars, glm::vec3(0.3f));

  // Venus parameters
  modelMatrixVenus = glm::mat4(1.0f);
  float venusOrbitAngle = 0.4f * currentTimeInSec;
  modelMatrixVenus = glm::translate(modelMatrixVenus, glm::vec3(glm::cos(venusOrbitAngle) * 8.0f, 0.0f, glm::sin(venusOrbitAngle) * 8.0f));
  modelMatrixVenus = glm::rotate(modelMatrixVenus, 0.9f * currentTimeInSec, glm::vec3(0.0f, 1.0f, 0.0f));
  modelMatrixVenus = glm::scale(modelMatrixVenus, glm::vec3(0.4f));

  // Additional planets
  // Jupiter
  modelMatrixJupiter = glm::mat4(1.0f);
  float jupiterOrbitAngle = 0.2f * currentTimeInSec;
  modelMatrixJupiter = glm::translate(modelMatrixJupiter, glm::vec3(glm::cos(jupiterOrbitAngle) * 20.0f, 0.0f, glm::sin(jupiterOrbitAngle) * 20.0f));
  modelMatrixJupiter = glm::rotate(modelMatrixJupiter, 0.5f * currentTimeInSec, glm::vec3(0.0f, 1.0f, 0.0f));
  modelMatrixJupiter = glm::scale(modelMatrixJupiter, glm::vec3(1.0f));

  // Saturn
  modelMatrixSaturn = glm::mat4(1.0f);
  float saturnOrbitAngle = 0.15f * currentTimeInSec;
  modelMatrixSaturn = glm::translate(modelMatrixSaturn, glm::vec3(glm::cos(saturnOrbitAngle) * 25.0f, 0.0f, glm::sin(saturnOrbitAngle) * 25.0f));
  modelMatrixSaturn = glm::rotate(modelMatrixSaturn, 0.4f * currentTimeInSec, glm::vec3(0.0f, 1.0f, 0.0f));
  modelMatrixSaturn = glm::scale(modelMatrixSaturn, glm::vec3(0.9f));

  // Uranus
  modelMatrixUranus = glm::mat4(1.0f);
  float uranusOrbitAngle = 0.1f * currentTimeInSec;
  modelMatrixUranus = glm::translate(modelMatrixUranus, glm::vec3(glm::cos(uranusOrbitAngle) * 30.0f, 0.0f, glm::sin(uranusOrbitAngle) * 30.0f));
  modelMatrixUranus = glm::rotate(modelMatrixUranus, 0.3f * currentTimeInSec, glm::vec3(0.0f, 1.0f, 0.0f));
  modelMatrixUranus = glm::scale(modelMatrixUranus, glm::vec3(0.7f));

  // Neptune
  modelMatrixNeptune = glm::mat4(1.0f);
  float neptuneOrbitAngle = 0.08f * currentTimeInSec;
  modelMatrixNeptune = glm::translate(modelMatrixNeptune, glm::vec3(glm::cos(neptuneOrbitAngle) * 35.0f, 0.0f, glm::sin(neptuneOrbitAngle) * 35.0f));
  modelMatrixNeptune = glm::rotate(modelMatrixNeptune, 0.3f * currentTimeInSec, glm::vec3(0.0f, 1.0f, 0.0f));
  modelMatrixNeptune = glm::scale(modelMatrixNeptune, glm::vec3(0.6f));

  // Mercury
  modelMatrixMercury = glm::mat4(1.0f);
  float mercuryOrbitAngle = 0.6f * currentTimeInSec;
  modelMatrixMercury = glm::translate(modelMatrixMercury, glm::vec3(glm::cos(mercuryOrbitAngle) * 5.0f, 0.0f, glm::sin(mercuryOrbitAngle) * 5.0f));
  modelMatrixMercury = glm::rotate(modelMatrixMercury, 1.0f * currentTimeInSec, glm::vec3(0.0f, 1.0f, 0.0f));
  modelMatrixMercury = glm::scale(modelMatrixMercury, glm::vec3(0.2f));
}



int main(int argc, char ** argv) {
  init(); // Your initialization code (user interface, OpenGL states, scene with geometry, material, lights, etc)
  while(!glfwWindowShouldClose(g_window)) {
    update(static_cast<float>(glfwGetTime()));
    render();
    glfwSwapBuffers(g_window);
    glfwPollEvents();
  }
  clear();
  return EXIT_SUCCESS;
}