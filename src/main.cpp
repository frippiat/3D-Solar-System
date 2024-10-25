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
glm::mat4 modelMatrixSun;
glm::mat4 modelMatrixEarth;
glm::mat4 modelMatrixMoon;

GLuint g_earthTexID;
GLuint g_moonTexID;


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
  } else if((action == GLFW_PRESS || action == GLFW_REPEAT) && key == GLFW_KEY_UP) {
    const glm::vec3 camPosition = g_camera.getPosition();
    g_camera.setPosition(glm::vec3(camPosition[0], camPosition[1], camPosition[2] - 0.1));
  } 
  // For DOWN arrow (move camera backward)
  else if((action == GLFW_PRESS || action == GLFW_REPEAT) && key == GLFW_KEY_DOWN) {
    const glm::vec3 camPosition = g_camera.getPosition();
    g_camera.setPosition(glm::vec3(camPosition[0], camPosition[1], camPosition[2] + 0.1));
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
  glLinkProgram(g_program); // The main GPU program is ready to be handle streams of polygons

  glUseProgram(g_program);
  // TODO: set shader variables, textures, etc.

  g_earthTexID = loadTextureFromFileToGPU("media/earth.jpg");
  glUniform1i(glGetUniformLocation(g_program, "material.albedoTex"), 0); // texture unit 0
  g_moonTexID = loadTextureFromFileToGPU("media/moon.jpg");
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

  g_camera.setPosition(glm::vec3(0.0, 0.0 , 20.0));
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

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.

  const glm::mat4 viewMatrix = g_camera.computeViewMatrix();
  const glm::mat4 projMatrix = g_camera.computeProjectionMatrix();
  const glm::vec3 camPosition = g_camera.getPosition();

  glUniformMatrix4fv(glGetUniformLocation(g_program, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMatrix)); // compute the view matrix of the camera and pass it to the GPU program
  glUniformMatrix4fv(glGetUniformLocation(g_program, "projMat"), 1, GL_FALSE, glm::value_ptr(projMatrix)); // compute the projection matrix of the camera and pass it to the GPU program
  glUniform3fv(glGetUniformLocation(g_program, "camPosition"), 1, glm::value_ptr(camPosition));


  glUniform1i(glGetUniformLocation(g_program, "isSun"), 1); //  indicates if fragment is on/in Sun or not should use a color instead of a texture
  glUniform3f(glGetUniformLocation(g_program, "objectColor"), 1.0f, 1.0f, 0.0f);  // yellow
  modelMatrixSun = glm::mat4(1.0f);
  modelMatrixSun = glm::scale(modelMatrixSun, glm::vec3(kSizeSun));
  glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrixSun));
  sphere->render();

  //Activate the texture for Earth
  glActiveTexture(GL_TEXTURE0); // Activate texture unit 0
  glBindTexture(GL_TEXTURE_2D, g_earthTexID); // Bind the Earth texture
  glUniform1i(glGetUniformLocation(g_program, "isSun"), 0); //  indicates if fragment is on/in Sun or not should use a color instead of a texture
  glUniform3f(glGetUniformLocation(g_program, "objectColor"), 0.0f, 1.0f, 0.0f); //magenta
  //modelMatrixEarth = glm::mat4(1.0f);
  //modelMatrixEarth = glm::translate(modelMatrixEarth, glm::vec3(kRadOrbitEarth, 0.0f, 0.0f));
  //modelMatrixEarth = glm::scale(modelMatrixEarth, glm::vec3(kSizeEarth));
  glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrixEarth));
  sphere->render();

  glActiveTexture(GL_TEXTURE0); // Activate texture unit 0
  glBindTexture(GL_TEXTURE_2D, g_moonTexID);
  glUniform1i(glGetUniformLocation(g_program, "isSun"), 0); //  indicates if fragment is on/in Sun or not should use a color instead of a texture
  glUniform3f(glGetUniformLocation(g_program, "objectColor"), 0.0f, 0.0f, 1.0f);  // cyan
  //modelMatrixMoon = glm::mat4(1.0f);
  //modelMatrixMoon = glm::translate(modelMatrixMoon, glm::vec3(kRadOrbitMoon+kRadOrbitEarth, 0.0f, 0.0f));
  //modelMatrixMoon = glm::scale(modelMatrixMoon, glm::vec3(kSizeMoon));
  glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrixMoon));
  sphere->render();

  /* TRIANGLES  
  glBindVertexArray(g_vao);     // activate the VAO storing geometry data
  glDrawElements(GL_TRIANGLES, g_triangleIndices.size(), GL_UNSIGNED_INT, 0); // Call for rendering: stream the current GPU geometry through the current GPU program
  */
 glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture
}

// Update any accessible variable based on the current time
void update(const float currentTimeInSec) {

    // Adjusted Orbital and Rotational Periods (relative time factors)
  float earthOrbitSpeed = 0.5f;   // Earth completes an orbit in 2 seconds
  float earthRotationSpeed = 1.0f; // Earth completes a rotation in 1 second (half the orbit time)
  float moonOrbitSpeed = 2.0f;     // Moon completes an orbit in 0.5 seconds (half of Earth’s rotation period)
  float moonRotationSpeed = 2.0f;  // Moon’s rotation period matches its orbital period

  // Earth's transformation (orbit around the Sun and rotation )
  modelMatrixEarth = glm::mat4(1.0f);
  float earthOrbitAngle = earthOrbitSpeed * currentTimeInSec;
  float earthX = glm::cos(earthOrbitAngle) * kRadOrbitEarth;
  float earthZ = glm::sin(earthOrbitAngle) * kRadOrbitEarth;
  modelMatrixEarth = glm::translate(modelMatrixEarth, glm::vec3(earthX, 0.0f, earthZ));

  // Apply Earth's axial tilt (23.5 degrees)
  modelMatrixEarth = glm::rotate(modelMatrixEarth, glm::radians(23.5f), glm::vec3(1.0f, 0.0f, 0.0f));

  // Earth's rotation around its axis
  modelMatrixEarth = glm::rotate(modelMatrixEarth, earthRotationSpeed * currentTimeInSec, glm::vec3(0.0f, 1.0f, 0.0f));
  modelMatrixEarth = glm::scale(modelMatrixEarth, glm::vec3(kSizeEarth));

  // Moon's transformation (orbit around Earth and rotation)
  modelMatrixMoon = glm::mat4(1.0f);
  float moonOrbitAngle = moonOrbitSpeed * currentTimeInSec;

  // Calculate Moon's position based on Earth's position
  glm::vec3 earthPosition = glm::vec3(modelMatrixEarth[3]); // Get Earth's current position
  modelMatrixMoon = glm::translate(modelMatrixMoon, earthPosition); // Start from Earth's position
  modelMatrixMoon = glm::translate(modelMatrixMoon, glm::vec3(glm::cos(moonOrbitAngle) * kRadOrbitMoon, 0.0f, glm::sin(moonOrbitAngle) * kRadOrbitMoon)); // Moon orbit around Earth
  modelMatrixMoon = glm::rotate(modelMatrixMoon, moonRotationSpeed * currentTimeInSec, glm::vec3(0.0f, 1.0f, 0.0f)); // Moon rotation to match orbit period
  modelMatrixMoon = glm::scale(modelMatrixMoon, glm::vec3(kSizeMoon));

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