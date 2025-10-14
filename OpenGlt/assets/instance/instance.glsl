#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;  
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aOffset;// Instance offset for instanced rendering
layout (location = 4) in vec3 aSize; // Instance scale for instanced rendering

out vec3 FragPos; 
out vec3 Normal; // Pass normal to fragment shader
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
vec3 position=vec3(aPos.x*aSize.x,aPos.y*aSize.y,aPos.z*aSize.z); // Scale the position by the instance size



    FragPos = vec3(model * vec4(position+aOffset, 1.0)); // Transform vertex position to world space
   Normal=mat3(transpose(inverse(model))) * aNormal; // Transform normal to world space
    // Apply the transformation matrix for rotation)


   gl_Position = projection * view *vec4(FragPos, 1.0); // Transform position to clip space
    TexCoord = aTexCoord;
}
