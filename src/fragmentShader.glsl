#version 330 core

in vec3 fNormal; // available per fragment, interpolated based on your drawing primitive
out vec4 color; // shader output: the color of this fragment

void main() 
{
    color = vec4(normalize(fNormal), 1.0); // output the normalized normal as color

    vec3 phongColor;
}