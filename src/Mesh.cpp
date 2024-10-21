#include "Mesh.hpp"

// Initializes the geometry buffer
void Mesh::init() {
    // TODO: Set up the VAO, VBOs, and IBO for the mesh geometry.
    // This should include binding the VAO, generating the buffer objects (glGenBuffers),
    // and uploading vertex data (glBufferData).

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

        // Generate and bind the Index Buffer Object (IBO) for triangle indices
        size_t indexBufferSize = sizeof(unsigned int)*m_triangleIndices.size();
        #ifdef _MY_OPENGL_IS_33_  //Irrelevant since Vincent's laptop has OpenGL version 4.6
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
void Mesh::render() {
    // TODO: Bind the VAO and issue the drawing commands using glDrawElements
    // or glDrawArrays depending on whether indexed drawing is used.
    // Ensure the appropriate shader program is bound before rendering.

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_triangleIndices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// Generates a unit sphere with the given resolution
std::shared_ptr<Mesh> Mesh::genSphere(const size_t resolution) {
    // TODO: Generate a unit sphere based on the given resolution.
    // This should calculate vertex positions, normals, and triangle indices.
    // Populate m_vertexPositions, m_vertexNormals, and m_triangleIndices.
    // Return a shared pointer to the newly created Mesh object.
    auto mesh = std::make_shared<Mesh>();

        const float radius = 1.0f; // Unit sphere
        const size_t latSegments = resolution; // Number of latitude segments
        const size_t lonSegments = resolution; // Number of longitude segments

        // Generate vertices and normals
        for (size_t lat = 0; lat <= latSegments; ++lat) {
            for (size_t lon = 0; lon <= lonSegments; ++lon) {
                float theta = lat * M_PI / latSegments; // latitude angle
                float phi = lon * 2.0f * M_PI / lonSegments; // longitude angle

                float x = radius * cosf(theta) * cosf(phi);
                float y = radius * cosf(theta) * sinf(phi);;
                float z = radius * sinf(theta);

                // Vertex position
                mesh->m_vertexPositions.push_back(x);
                mesh->m_vertexPositions.push_back(y);
                mesh->m_vertexPositions.push_back(z);

                // Vertex normal (same as position for a unit sphere)
                mesh->m_vertexNormals.push_back(x/radius);
                mesh->m_vertexNormals.push_back(y/radius);
                mesh->m_vertexNormals.push_back(z/radius);
            }
        }

        // Generate indices for triangle strips
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
