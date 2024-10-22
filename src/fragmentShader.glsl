#version 330 core

<<<<<<< HEAD
uniform vec3 camPosition;  // Camera position
uniform vec3 lightPosition;
=======
uniform vec3 camPos;  // Camera position
>>>>>>> ebea76ab44992a002ee8e75716a5f43bf48c6071
in vec3 fPosition;    // Fragment position in world space
in vec3 fNormal;      // Fragment normal in world space

out vec4 color;       // Output fragment color

void main() 
{
    vec3 n = normalize(fNormal); // Normalize the normal vector
    vec3 l = normalize(vec3(1.0, 1.0, 1.0)); // Light direction (hardcoded)

    // Calculate view vector (v), pointing from fragment position to camera
<<<<<<< HEAD
    vec3 v = normalize(camPosition - fPosition);
=======
    vec3 v = normalize(camPos - fPosition);
>>>>>>> ebea76ab44992a002ee8e75716a5f43bf48c6071

    // Calculate reflection vector (r) using reflect() function
    vec3 r = reflect(-l, n);  // reflect expects the incoming light vector, so we negate l

    // Ambient lighting (constant low-intensity light)
    vec3 ambient = vec3(0.2,0.2, 0.2); // Low ambient light intensity

    // Diffuse lighting using Lambert's cosine law
    float diff = max(dot(n, l), 0.0); // Dot product between light direction and normal
    vec3 diffuse = diff * vec3(0.8, 0.8, 0.8); // White light for diffuse component

    // Specular lighting using the Blinn-Phong reflection model
    float shininess = 32.0; // Shininess factor for specular highlight
    float max_vr = max(dot(v, r), 0.0);
    float spec = pow(max_vr, shininess); // Phong specular highlight
    vec3 specular = spec * vec3(1.0, 1.0, 1.0); // White specular highlights

    // Combine all lighting components (ambient + diffuse + specular)
    color = vec4(ambient + specular+ diffuse, 1.0); // Final color with alpha = 1
<<<<<<< HEAD
}
=======
}
>>>>>>> ebea76ab44992a002ee8e75716a5f43bf48c6071
