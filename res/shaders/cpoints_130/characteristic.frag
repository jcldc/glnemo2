#version 130

uniform vec3 color;
uniform bool second_pass;

void main() {
    if(second_pass)
        gl_FragColor = vec4(1, 0.3, 0.3, 1.0f);
    else
        gl_FragColor = vec4(color, 1.0f);
}