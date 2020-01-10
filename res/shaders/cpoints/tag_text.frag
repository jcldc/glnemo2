#version 130
in vec2 TexCoords;

uniform sampler2D tex;
uniform vec3 color;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(tex, TexCoords).r);
    gl_FragColor = vec4(color, 1.0) * sampled;
}