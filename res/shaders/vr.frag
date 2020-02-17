#version 130

uniform sampler2D tex;
in vec2 UV;

void main() {
//    vec2 d = vec2(
//        cos(UV[0])*cos(UV[1]),
//        sin(UV[0])*cos(UV[1])
//    );
//    gl_FragColor = texture(tex, d);
    gl_FragColor = texture(tex, UV);
}