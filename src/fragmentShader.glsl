#version 330 core

in vec3 fPosition;    // Fragment position in world space
in vec3 fNormal;      // Fragment normal in world space

out vec4 color;       // Output fragment color

uniform vec3 camPosition;  // Camera position
uniform vec3 objectColor; //Color the object
//uniform int isSun;         // is the object the sun or not

void main() 
{
    vec3 n = normalize(fNormal); // Normalize the normal vector
    vec3 lightPosition = vec3(0.0, 0.0, 0.0);
    vec3 l = normalize(lightPosition-fPosition); // Light direction (hardcoded)

    // Calculate view vector (v), pointing from fragment position to camera
    vec3 v = normalize(camPosition - fPosition);

    // Calculate reflection vector (r) using reflect() function
    vec3 r = reflect(-l, n);  // reflect expects the incoming light vector, so we negate l

    // Ambient lighting (constant low-intensity light)
    vec3 ambient = vec3(0.2,0.2, 0.2); // Low ambient light intensity


    // Diffuse lighting using Lambert's cosine law
    vec3 lightSourceColor=vec3(0.9, 0.9, 0.2);
    float diff = max(dot(n, l), 0.0); // Dot product between light direction and normal
    vec3 diffuse = diff * lightSourceColor; // White light for diffuse component

    // Specular lighting using the Blinn-Phong reflection model
    float shininess = 32.0; // Shininess factor for specular highlight
    float max_vr = max(dot(v, r), 0.0);
    float spec = pow(max_vr, shininess); // Phong specular highlight
    vec3 specular = spec * vec3(1.0, 1.0, 1.0); // White specular highlights

    // Combine all lighting components (ambient + diffuse + specular)
    vec3 finalColor = ambient+specular+ diffuse;
    color = vec4(objectColor*finalColor, 1.0); // Final color with alpha = 1
}
