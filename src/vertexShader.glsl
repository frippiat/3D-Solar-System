#version 330 core

layout(location=0) in vec3 vPosition; // input vertex positions
layout(location=1) in vec3 vNormal; // input vertex normals
uniform mat4 viewMat, projMat;
out vec3 fNormal; // output normal to the next stage

void main() 
{
    gl_Position = projMat * viewMat * vec4(vPosition, 1.0); // mandatory to rasterize properly
    fNormal = vNormal; // pass the vertex normal to the next stage
}
