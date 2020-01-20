#version 130
in vec2 TexCoords;

uniform sampler2D tex;
uniform vec3 color;

void main()
{
    gl_FragColor = vec4(color, texture(tex, TexCoords).a);
}