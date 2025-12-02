#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

// 🔴 OUTPUTS ไปยัง Fragment Shader
out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;
out vec3 Ambient;

// 🔴 UNIFORMS จาก C++
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // 1. ตำแหน่ง (ใน World Space) และ Normal
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal; 
    
    // 2. UV Coords
    TexCoords = aTexCoords;

    // 3. Ambient Light (ตั้งค่าคงที่ เพราะกำแพงควรเรืองแสงด้วย Emissive Texture)
    Ambient = vec3(1.0); 
    
    // 4. ตำแหน่งสุดท้ายบนหน้าจอ (รวม Model, View, Projection)
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}