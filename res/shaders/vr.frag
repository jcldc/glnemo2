#version 130

uniform samplerCube tex;
in vec2 UV;
out vec4 out_color;

void main() {
    vec3 d = vec3(
        cos(UV[0])*cos(UV[1]),
        sin(UV[1]),
        sin(UV[0])*cos(UV[1])
    );
    out_color = texture(tex, d);
//    out_color = texture(tex, UV);
}