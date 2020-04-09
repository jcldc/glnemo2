#version 120

uniform vec3 color;
uniform bool second_pass;
uniform vec3 selected_color;

void main() {
    if(second_pass)
        gl_FragColor = vec4(selected_color, 1.0f);
    else
        gl_FragColor = vec4(color, 1.0f);
}