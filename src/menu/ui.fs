#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D uiTexture;
uniform vec3 color;
uniform float alpha;

void main()
{
    vec4 texColor = texture(uiTexture, TexCoords);
    FragColor = vec4(color, alpha) * texColor;
}