#version 330 core
layout(location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 model_view_matrix;
uniform mat4 proj_matrix;
uniform vec3 point_center;
uniform float offset;
uniform int angle;

const float PI = 3.14159265359;

void main()
{
    vec3 centerWorld = (model_view_matrix*vec4(point_center, 1.0)).xyz;

    float angle_rad = float(angle)/360.0f*PI*2.0f;
    vec3 text_position = vec3(cos(angle_rad)*offset, sin(angle_rad)*offset, 0);

    vec3 billboard_vertex_pos = centerWorld + vec3( vertex.xy, 0.0) + text_position;

    gl_Position = proj_matrix*vec4(billboard_vertex_pos, 1.0);

    TexCoords = vertex.zw;
}
