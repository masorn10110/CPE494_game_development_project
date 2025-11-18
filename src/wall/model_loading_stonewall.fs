#version 330 core
out vec4 FragColor;

// 🔴 INPUTS จาก Vertex Shader
in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec3 Ambient;

// 🔴 UNIFORMS จาก C++
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float alphaFade;
uniform float emissionIntensity;

uniform sampler2D texture_emissive;

void main()
{
    // 1. ดึงสีจาก Texture (เราสมมติว่า Emissive Texture มีสีไฟ)
    vec4 texColor = texture(texture_emissive, TexCoords);
    
    // 2. คำนวณ Emissive Component
    // ใช้สีจาก texture เป็นฐาน แล้วคูณด้วยความเข้มที่ควบคุมจาก C++
    vec3 emissiveLight = texColor.rgb * emissionIntensity; 
    
    // 3. คำนวณสีสุดท้าย
    // เนื่องจากกำแพงนี้เรืองแสงเอง เราอาจจะไม่ต้องคำนวณแสงแบบ Diffuse/Specular ที่ซับซ้อน
    // สีสุดท้าย = สี Texture (Emissive) * Ambient (1.0) + EmissiveLight
    vec3 finalColor = texColor.rgb * Ambient + emissiveLight;
    
    // 4. ส่งออกสีและ Alpha
    // ใช้ alphaFade ควบคุมความโปร่งใส
    FragColor = vec4(finalColor, texColor.a * alphaFade);
}