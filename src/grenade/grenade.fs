#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform vec4 color;
uniform float time;

void main()
{
    // Convert UV (0–1) → (-1 to +1)
    vec2 uv = TexCoord * 2.0 - 1.0;
    float dist = length(uv);

    // 🔥 Soft circular mask
    float alpha = smoothstep(1.0, 0.0, dist);

    // 🔥 Flicker
    float flicker = 0.6 + 0.4 * sin(time * 12.0 + dist * 8.0);

    // 🔥 Fire color ramp
    vec3 fire = mix(
        vec3(1.0, 0.2, 0.0),   // red
        vec3(1.0, 1.0, 0.4),   // yellow
        1.0 - dist
    );

    FragColor = vec4(fire * flicker, alpha * color.a);

    // ❌ kill square edges completely
    if (alpha < 0.01)
        discard;
}
