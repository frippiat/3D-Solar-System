#version 330 core

layout(location=0) in vec3 vPosition; // input vertex positions
layout(location=1) in vec3 vNormal; // input vertex normals
layout(location=2) in vec2 vTexCoord; //input texture coordinates

uniform mat4 viewMat, projMat,modelMatrix;

out vec3 fNormal;
out vec3 fPosition;
out vec2 fTexCoord;

void main() 
{
    vec4 worldPosition = modelMatrix * vec4(vPosition, 1.0);
    fPosition = vec3(worldPosition); 
    fNormal =mat3(transpose(inverse(modelMatrix))) * vNormal;  // NOG VERFIEER OF DIT CORRECT BEREKEND IS
    gl_Position = projMat * viewMat * worldPosition;
    fTexCoord=vTexCoord;
}



