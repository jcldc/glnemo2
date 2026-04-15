#version 330 core
in vec2 TexCoords;

uniform sampler2D tex;
uniform vec3 color;

out vec4 frag_color;

void main()
{
    frag_color = vec4(color, texture(tex, TexCoords).a);
}