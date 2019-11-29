#version 130

uniform int nb_vertices;
uniform float radius;
uniform mat4 modelviewMatrix;
uniform mat4 projMatrix;
uniform vec3 CameraUp_worldspace;
uniform vec3 CameraRight_worldspace;
uniform vec3 point_center;

float PI = 3.14159265359;

void main()
{
    float angle = (2*PI/nb_vertices)*gl_VertexID;

    vec3 position = vec3(radius*cos(angle),radius*sin(angle),0.f);

    vec3 center = (modelviewMatrix*vec4(point_center, 1.0f)).xyz;

    vec3 billboard_vertex_pos =
    center
    + CameraRight_worldspace * position.x
    + CameraUp_worldspace * position.y;

    gl_Position = projMatrix*vec4(billboard_vertex_pos, 1.0f);

}
