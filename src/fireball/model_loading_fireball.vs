#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;
out vec3 Ambient;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    TexCoords = aTexCoords;

    float wobble = sin(time * 5.0 + aPos.x * 2.0 + aPos.y * 3.0) * 0.05; 
    vec3 displacedPos = aPos + aNormal * wobble; // เลื่อนตาม Normal

    Ambient = vec3(1.0);
    
    gl_Position = projection * view * model * vec4(displacedPos, 1.0);
}
