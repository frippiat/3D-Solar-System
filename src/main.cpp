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

glm::mat4 modelSun, modelEarth, modelMoon, modelMars, modelVenus, modelUranus, modelSaturn, modelNeptune, modelJupiter, modelMercury;

GLuint g_earthTexID, g_moonTexID, g_marsTexID, g_venusTexID, g_uranusTexID, g_saturnTexID, g_neptuneTexID, g_jupiterTexID, g_mercuryTexID;

class Mesh {
public:
    // Initializes the geometry buffer
void init() {
    // Set up the VAO, VBOs, and IBO for the mesh geometry

    // Create and bind the Vertex Array Object (VAO)
        #ifdef _MY_OPENGL_IS_33_
        glGenVertexArrays(1, &m_vao); // If your system doesn't support OpenGL 4.5, you should use this instead of glCreateVertexArrays.
        #else
        glCreateVertexArrays(1, &m_vao);
        #endif

        glBindVertexArray(m_vao);

        // Generate and bind the Vertex Buffer Object (VBO) for positions
        glGenBuffers(1, &m_posVbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_posVbo);
        glBufferData(GL_ARRAY_BUFFER, m_vertexPositions.size() * sizeof(float), m_vertexPositions.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0); // position is index 0 in the shader

        // Generate and bind the Vertex Buffer Object (VBO) for normals
        glGenBuffers(1, &m_normalVbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_normalVbo);
        glBufferData(GL_ARRAY_BUFFER, m_vertexNormals.size() * sizeof(float), m_vertexNormals.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1); // normals are index 1 in the shader

        glGenBuffers(1, &m_texCoordVbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_texCoordVbo);
        glBufferData(GL_ARRAY_BUFFER, m_vertexTexCoords.size() * sizeof(float), m_vertexTexCoords.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0); // Texture coordinates are at index 2
        glEnableVertexAttribArray(2);

        // Generate and bind the Index Buffer Object (IBO) for triangle indices
        size_t indexBufferSize = sizeof(unsigned int)*m_triangleIndices.size();
        #ifdef _MY_OPENGL_IS_33_  //Should be irrelevant since Vincent's laptop has OpenGL version 4.6
            glGenBuffers(1, &m_ibo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, m_triangleIndices.data(), GL_STATIC_DRAW);
        #else
            glCreateBuffers(1, &m_ibo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
            glNamedBufferStorage(m_ibo, indexBufferSize, m_triangleIndices.data(), GL_DYNAMIC_STORAGE_BIT);
        #endif
            // Unbind the VAO for now
            glBindVertexArray(0);
    }
    
    // Called in the main rendering loop to render the mesh
void render() {
        // Bind the VAO and issue the drawing commands
        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_triangleIndices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    
    // Generates a unit sphere with the given resolution
static std::shared_ptr<Mesh> genSphere(const size_t resolution) {
      auto mesh = std::make_shared<Mesh>();

          const float radius = 1.0f; // Unit sphere

          const size_t latSegments = resolution; // Number of latitude segments
          const size_t lonSegments = resolution; // Number of longitude segments

          // Generate vertices and normals
          for (size_t lat = 0; lat <= latSegments; ++lat) {
              for (size_t lon = 0; lon <= lonSegments; ++lon) {
                  float theta = M_PI/2.0-lat *M_PI / latSegments;  // latitude angle
                  float phi = lon * 2.0f * M_PI / lonSegments; // longitude angle

                  float x = radius * cosf(theta) * cosf(phi);
                  float y = radius * cosf(theta) * sinf(phi);
                  float z = radius * sinf(theta);

                  // Vertex position
                  mesh->m_vertexPositions.push_back(x);
                  mesh->m_vertexPositions.push_back(z);
                  mesh->m_vertexPositions.push_back(y);

                  // Vertex normal (same as position for a unit sphere)
                  mesh->m_vertexNormals.push_back(x/radius);
                  mesh->m_vertexNormals.push_back(z/radius);
                  mesh->m_vertexNormals.push_back(y/radius);

                  // Texture coordinates
                  float u = (float)lon / lonSegments;
                  float v = (float)lat / latSegments;
                  mesh->m_vertexTexCoords.push_back(u);
                  mesh->m_vertexTexCoords.push_back(v);
              }
          }

          // Generate indices for triangle strips.
          for (size_t lat = 0; lat < latSegments; ++lat) {
              for (size_t lon = 0; lon < lonSegments; ++lon) {
                  unsigned int first = (lat * (lonSegments + 1)) + lon;
                  unsigned int second = first + lonSegments + 1;

                  // First triangle
                  mesh->m_triangleIndices.push_back(first);
                  mesh->m_triangleIndices.push_back(second);
                  mesh->m_triangleIndices.push_back(first + 1);

                  // Second triangle
                  mesh->m_triangleIndices.push_back(second);
                  mesh->m_triangleIndices.push_back(second + 1);
                  mesh->m_triangleIndices.push_back(first + 1);
              }
          }

          return mesh;
        }

private:
    // Vertex positions for the mesh
    std::vector<float> m_vertexPositions;

    // Vertex normals for the mesh
    std::vector<float> m_vertexNormals;

    // Triangle indices for the mesh
    std::vector<unsigned int> m_triangleIndices;

    std::vector<float> m_vertexTexCoords;  // Texture coordinates

    // OpenGL-related buffers
    GLuint m_vao = 0;        // Vertex Array Object (VAO)
    GLuint m_posVbo = 0;     // Vertex Buffer Object (VBO) for the positions
    GLuint m_normalVbo = 0;  // Vertex Buffer Object (VBO) for the normals
    GLuint m_ibo = 0;        // Index Buffer Object (IBO)
    GLuint m_texCoordVbo = 0; //Vertex Buffer Object (VBO) for the texture coordinates
};
//Sphere mesh
std::shared_ptr<Mesh> sphere;


// Basic camera model
class Camera {
public:
  inline float getFov() const { return m_fov; }
  inline void setFoV(const float f) { m_fov = f; }
  inline float getAspectRatio() const { return m_aspectRatio; }
  inline void setAspectRatio(const float a) { m_aspectRatio = a; }
  inline float getNear() const { return m_near; }
  inline void setNear(const float n) { m_near = n; }
  inline float getFar() const { return m_far; }
  inline void setFar(const float n) { m_far = n; }
  inline void setPosition(const glm::vec3 &p) { m_pos = p; }
  inline glm::vec3 getPosition() { return m_pos; }

  inline glm::mat4 computeViewMatrix() const {
    return glm::lookAt(m_pos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
  }

  // Returns the projection matrix stemming from the camera intrinsic parameter.
  inline glm::mat4 computeProjectionMatrix() const {
    return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_near, m_far);
  }

private:
  glm::vec3 m_pos = glm::vec3(0, 0, 0);
  float m_fov = 45.f;        // Field of view, in degrees
  float m_aspectRatio = 1.f; // Ratio between the width and the height of the image
  float m_near = 0.1f; // Distance before which geometry is excluded from the rasterization process
  float m_far = 10.f; // Distance after which the geometry is excluded from the rasterization process
};
Camera g_camera;


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
  modelSun = glm::mat4(1.0f);
  modelSun = glm::scale(modelSun, glm::vec3(kSizeSun));
  glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelSun));
  sphere->render();

  glActiveTexture(GL_TEXTURE0);
  glUniform1i(glGetUniformLocation(g_program, "isSun"), 0);

  // Earth rendering
  glBindTexture(GL_TEXTURE_2D, g_earthTexID);
  glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelEarth));
  sphere->render();

  // Moon rendering
  glBindTexture(GL_TEXTURE_2D, g_moonTexID);
  glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMoon));
  sphere->render();

  // Mars rendering
  glBindTexture(GL_TEXTURE_2D, g_marsTexID);
  glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMars));
  sphere->render();

  // Venus rendering
  glBindTexture(GL_TEXTURE_2D, g_venusTexID);
  glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelVenus));
  sphere->render();

  // Jupiter rendering
  glBindTexture(GL_TEXTURE_2D, g_jupiterTexID);
  glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelJupiter));
  sphere->render();

  // Saturn rendering
  glBindTexture(GL_TEXTURE_2D, g_saturnTexID);
  glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelSaturn));
  sphere->render();

  // Uranus rendering
  glBindTexture(GL_TEXTURE_2D, g_uranusTexID);
  glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelUranus));
  sphere->render();

  // Neptune rendering
  glBindTexture(GL_TEXTURE_2D, g_neptuneTexID);
  glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelNeptune));
  sphere->render();

  // Mercury rendering
  glBindTexture(GL_TEXTURE_2D, g_mercuryTexID);
  glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMercury));
  sphere->render();


  glBindTexture(GL_TEXTURE_2D, 0);
}


void update(const float currentTimeInSec) {
  // Constants for orbital and rotational periods
  // Speeds of rotation and orbit for Earth and Moon
  float speedEarthRotation = 1.0f;
  float speedEarthOrbit = 0.5f * speedEarthRotation;
  float speedMoonRotation = 2.0f * speedEarthRotation;
  float speedMoonOrbit = speedMoonRotation;

  // Initialize Earth's transformation matrix
  modelEarth = glm::mat4(1.0f);
  glm::mat4 translateEarth = glm::translate(glm::mat4(1.0f), glm::vec3(kRadOrbitEarth * cos(0.3f * currentTimeInSec), 0.0f, kRadOrbitEarth * sin(0.3f * currentTimeInSec)));
  glm::mat4 scaleEarth = glm::scale(glm::mat4(1.0f), glm::vec3(kSizeEarth));
  glm::mat4 rotateEarth = glm::rotate(glm::mat4(1.0f), 0.6f * currentTimeInSec, glm::vec3(sin(glm::radians(23.5f)), cos(glm::radians(23.5f)), 0.0f));
  modelEarth = translateEarth * scaleEarth * rotateEarth;

  // Initialize Moon's transformation matrix relative to Earth's position
  modelMoon = glm::mat4(1.0f);
  glm::mat4 translateMoon = glm::translate(glm::mat4(1.0f), glm::vec3(kRadOrbitMoon * cos(0.6f * currentTimeInSec), 0.0f, kRadOrbitMoon * sin(0.6f * currentTimeInSec)));
  glm::mat4 rotateMoon = glm::rotate(glm::mat4(1.0f), 0.6f * currentTimeInSec, glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 scaleMoon = glm::scale(glm::mat4(1.0f), glm::vec3(kSizeMoon));
  modelMoon = translateEarth * translateMoon * rotateMoon * scaleMoon;

  // Mars parameters
  modelMars = glm::mat4(1.0f);
  float marsOrbitAngle = 0.3f * currentTimeInSec;
  modelMars = glm::translate(modelMars, glm::vec3(glm::cos(marsOrbitAngle) * 15.0f, 0.0f, glm::sin(marsOrbitAngle) * 15.0f));
  modelMars = glm::rotate(modelMars, 0.8f * currentTimeInSec, glm::vec3(0.0f, 1.0f, 0.0f));
  modelMars = glm::scale(modelMars, glm::vec3(0.3f));

  // Venus parameters
  modelVenus = glm::mat4(1.0f);
  float venusOrbitAngle = 0.4f * currentTimeInSec;
  modelVenus = glm::translate(modelVenus, glm::vec3(glm::cos(venusOrbitAngle) * 8.0f, 0.0f, glm::sin(venusOrbitAngle) * 8.0f));
  modelVenus = glm::rotate(modelVenus, 0.9f * currentTimeInSec, glm::vec3(0.0f, 1.0f, 0.0f));
  modelVenus = glm::scale(modelVenus, glm::vec3(0.4f));

  // Jupiter
  modelJupiter = glm::mat4(1.0f);
  float jupiterOrbitAngle = 0.2f * currentTimeInSec;
  modelJupiter = glm::translate(modelJupiter, glm::vec3(glm::cos(jupiterOrbitAngle) * 20.0f, 0.0f, glm::sin(jupiterOrbitAngle) * 20.0f));
  modelJupiter = glm::rotate(modelJupiter, 0.5f * currentTimeInSec, glm::vec3(0.0f, 1.0f, 0.0f));
  modelJupiter = glm::scale(modelJupiter, glm::vec3(1.0f));

  // Saturn
  modelSaturn = glm::mat4(1.0f);
  float saturnOrbitAngle = 0.15f * currentTimeInSec;
  modelSaturn = glm::translate(modelSaturn, glm::vec3(glm::cos(saturnOrbitAngle) * 25.0f, 0.0f, glm::sin(saturnOrbitAngle) * 25.0f));
  modelSaturn = glm::rotate(modelSaturn, 0.4f * currentTimeInSec, glm::vec3(0.0f, 1.0f, 0.0f));
  modelSaturn = glm::scale(modelSaturn, glm::vec3(0.9f));

  // Uranus
  modelUranus = glm::mat4(1.0f);
  float uranusOrbitAngle = 0.1f * currentTimeInSec;
  modelUranus = glm::translate(modelUranus, glm::vec3(glm::cos(uranusOrbitAngle) * 30.0f, 0.0f, glm::sin(uranusOrbitAngle) * 30.0f));
  modelUranus = glm::rotate(modelUranus, 0.3f * currentTimeInSec, glm::vec3(0.0f, 1.0f, 0.0f));
  modelUranus = glm::scale(modelUranus, glm::vec3(0.7f));

  // Neptune
  modelNeptune = glm::mat4(1.0f);
  float neptuneOrbitAngle = 0.08f * currentTimeInSec;
  modelNeptune = glm::translate(modelNeptune, glm::vec3(glm::cos(neptuneOrbitAngle) * 35.0f, 0.0f, glm::sin(neptuneOrbitAngle) * 35.0f));
  modelNeptune = glm::rotate(modelNeptune, 0.3f * currentTimeInSec, glm::vec3(0.0f, 1.0f, 0.0f));
  modelNeptune = glm::scale(modelNeptune, glm::vec3(0.6f));

  // Mercury
  modelMercury = glm::mat4(1.0f);
  float mercuryOrbitAngle = 0.6f * currentTimeInSec;
  modelMercury = glm::translate(modelMercury, glm::vec3(glm::cos(mercuryOrbitAngle) * 5.0f, 0.0f, glm::sin(mercuryOrbitAngle) * 5.0f));
  modelMercury = glm::rotate(modelMercury, 1.0f * currentTimeInSec, glm::vec3(0.0f, 1.0f, 0.0f));
  modelMercury = glm::scale(modelMercury, glm::vec3(0.2f));
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




