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
    // Apply per-instance scale and translation in world space (optional)
    vec3 size = (useInstancing == 1) ? iSize : vec3(1.0);
    vec3 offset = (useInstancing == 1) ? iPos : vec3(0.0);
    vec3 scaled = aPos * size;
    vec3 world = vec3(model * vec4(scaled + offset, 1.0));
    FragPos = world;

    Normal = mat3(transpose(inverse(model))) * aNormal; // Approximate normal transform

    gl_Position = projection * view * vec4(FragPos, 1.0);
    TexCoord = aTexCoord;
}
