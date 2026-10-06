#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;  
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 iPos;   // per-instance world position
layout (location = 4) in vec3 iSize;  // per-instance scale

out vec3 FragPos; 
out vec3 Normal; // Pass normal to fragment shader
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int useInstancing; // 1 when drawing instanced, else 0

void main()
{
    vec4 worldPos;
    
    if (useInstancing == 1) {
        // Apply per-instance scale and translation in world space
        vec3 scaled = aPos * iSize;
        worldPos = model * vec4(scaled + iPos, 1.0);
    } else {
        // Standard transformation (model matrix already contains translation and scale)
        worldPos = model * vec4(aPos, 1.0);
    }
    
    FragPos = vec3(worldPos);

    Normal = mat3(transpose(inverse(model))) * aNormal; // Approximate normal transform

    gl_Position = projection * view * worldPos;
    TexCoord = aTexCoord;
}
