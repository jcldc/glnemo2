#version 130

in vec2 vertexUV;
out vec2 UV;
float PI = 3.14159265359;

void main() {
    vec2 quad[4] = vec2[](
    vec2(1.f, 1.f),
    vec2(1.f, -1.f),
    vec2(-1.f, 1.f),
    vec2(-1.f, -1.f)
    );
    vec2 vertexUV[4] = vec2[](
    vec2(PI, PI/2),
    vec2(PI, -PI/2),
    vec2(-PI, PI/2),
    vec2(-PI, -PI/2)
    );
    UV = vertexUV[gl_VertexID];
    gl_Position = vec4(quad[gl_VertexID], 0.0, 1.0);
}
