#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec3 Ambient;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float emissionPulse; 
uniform sampler2D texture_diffuse1;

void main()
{
    // สี Texture ปกติ
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    
    // 🔴 สีเปลวไฟฐาน: ส้ม/เหลือง (ปรับได้ตามต้องการ)
    vec3 fireColor = vec3(1.0, 0.5, 0.0); 

    // 🔴 คำนวณ Emissive Component: ใช้ Pulse * FireColor
    // ไฟจะสว่างขึ้นเมื่อ emissionPulse ใกล้ 1.0
    vec3 emissiveLight = fireColor * emissionPulse * 1.5; 
    
    vec3 calculatedColor = texColor.rgb * Ambient; // ส่วน Diffuse/Ambient Light (ถ้ามี)
    vec3 finalColor = calculatedColor + emissiveLight;
    
    FragColor = vec4(finalColor, texColor.a);
}
