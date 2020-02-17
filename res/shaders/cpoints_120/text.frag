#version 120
varying vec2 TexCoords;

uniform sampler2D tex;
uniform vec3 color;

void main()
{
    gl_FragColor = vec4(color, texture2D(tex, TexCoords).a);
}