#version 330 core

in vec3 fPosition;    // Fragment position in world space
in vec3 fNormal;      // Fragment normal in world space

out vec4 color;       // Output fragment color

uniform vec3 camPosition;  // Camera position
uniform vec3 objectColor; //Color the object
uniform int isSun;         // is the object the sun or not

void main() 
{
    if (isSun == 1) {
        color = vec4(objectColor, 1.0);  // Just render Sun's base color
        return;
    }

    vec3 n = normalize(fNormal); // Normalize the normal vector
    vec3 lightPosition = vec3(0.0, 0.0, 0.0);
    vec3 l = normalize(lightPosition-fPosition); // Light direction (hardcoded)

    // Ambient lighting (constant low-intensity light)
    vec3 ambient = vec3(0.2,0.2, 0.2)*objectColor; // Low ambient light intensity


    // Diffuse lighting using Lambert's cosine law
    vec3 lightSourceColor=vec3(1.0 ,1.0,1.0);
    float diff = max(dot(n, l), 0.0);
    vec3 diffuse = diff * lightSourceColor; // White light for diffuse component


    // Specular lighting using the Phong reflection model
    float shininess = 32.0;     // Shininess factor for specular highlight 
    vec3 v = normalize(camPosition - fPosition);       // Calculate view vector (v), pointing from fragment position to camera
    vec3 r = reflect(-l, n);        // Reflect expects the incoming light vector, so we negate l // Calculate reflection vector (r) using reflect() function     
    float spec = pow(max(dot(v, r), 0.0), shininess);       // Calculate specular lighting
    vec3 specular = spec * vec3(1.0, 1.0, 1.0); // White specular highlights

    // Combine all lighting components (ambient + diffuse + specular)
    vec3 finalColor = ambient+specular+ diffuse;
    color = vec4(objectColor*finalColor, 1.0); // Final color with alpha = 1

    return;
}
