#version 130

uniform int nb_vertices;
uniform mat4 modelviewMatrix; // TODO change name
uniform mat4 projMatrix;
uniform bool is_filled;
uniform float fill_ratio;

in vec3 point_center;
in float radius;

float PI = 3.14159265359;

void main()
{
    float angle;
    vec3 relativePos;
    if(gl_VertexID % 2 == 1 && !is_filled){
        angle = 2*PI/nb_vertices*(gl_VertexID - 1)-PI/4;
        float inner_radius = radius-radius*fill_ratio;
        relativePos = vec3(inner_radius*cos(angle),inner_radius*sin(angle), 0);
    }
    else{
        angle = 2*PI/nb_vertices*gl_VertexID-PI/4;
        relativePos = vec3(radius*cos(angle),radius*sin(angle), 0);
    }
    vec3 centerWorld = (modelviewMatrix*vec4(point_center, 1)).xyz;
    vec3 billboard_vertex_pos = centerWorld + relativePos;

    gl_Position = projMatrix*vec4(billboard_vertex_pos, 1);
}
