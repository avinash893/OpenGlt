#version 330 core 

out vec4 FragColor;  // Changed from Fragcolor to FragColor for convention

uniform vec3 lightColor;  // Also changed variable name for consistency

void main() {
    // Set the fragment color to the light color
    FragColor = vec4(lightColor, 1.0);
}