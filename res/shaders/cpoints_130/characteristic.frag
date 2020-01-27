#version 130

uniform vec3 color;
uniform bool second_pass;

out vec4 frag_color;

void main() {
    if(second_pass)
        frag_color = vec4(1, 0.3, 0.3, 1.0f);
    else
        frag_color = vec4(color, 1.0f);
}