#version 130
uniform vec3 color;
uniform bool second_pass;

flat in float selected;

void main() {
    if(second_pass)
        if(selected > 0.5)
            gl_FragColor = vec4(1, 0.3, 0.3, 1.0f);
        else
            discard;
    else
        gl_FragColor = vec4(color, 1.0f);
}