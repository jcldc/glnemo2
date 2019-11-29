#version 130

uniform int nb_vertices;
uniform float radius;
uniform mat4 modelviewMatrix;
uniform mat4 projMatrix;
uniform vec3 point_center;
uniform bool is_annulus;

float PI = 3.14159265359;

void main()
{
    float angle = (2*PI/nb_vertices)*gl_VertexID;
    vec3 position;

    if(is_annulus && gl_VertexID % 2 == 0)
        position = vec3((radius-.1)*cos(angle),(radius-.1)*sin(angle),0.f);
    else
        position = vec3(radius*cos(angle),radius*sin(angle),0.f);

    vec3 centerWorld = (modelviewMatrix*vec4(point_center, 1.0f)).xyz;

    vec3 billboard_vertex_pos = centerWorld + position.xzz + position.zyz; // z = 0

    gl_Position = projMatrix*vec4(billboard_vertex_pos, 1.0f);

}
