#ifndef MESH_HPP
#define MESH_HPP

#include <vector>
#include <memory> // for std::shared_ptr
#include <cstddef> // for size_t
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Mesh {
public:
    // Initializes the geometry buffer
    void init();
    
    // Called in the main rendering loop to render the mesh
    void render();
    
    // Generates a unit sphere with the given resolution (default is 16)
    static std::shared_ptr<Mesh> genSphere(const size_t resolution = 16);

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
    GLuint m_posVbo = 0;     // Vertex Buffer Object (VBO) for positions
    GLuint m_normalVbo = 0;  // Vertex Buffer Object (VBO) for normals
    GLuint m_ibo = 0;        // Index Buffer Object (IBO)
    GLuint m_texCoordVbo = 0; //Vertex Buffer Object (VBO) for texture coordinates
};

#endif // MESH_HPP