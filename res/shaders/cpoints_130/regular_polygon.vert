#version 130

uniform int nb_vertices;
uniform mat4 model_view_matrix;
uniform mat4 proj_matrix;
uniform float fill_ratio;
uniform bool second_pass;
uniform ivec2 screen_dims;

in vec3 point_center;
in float radius;

float PI = 3.14159265359;

void main()
{
    float angle;
    vec3 relativePos;
    vec4 vpos;
    if (mod(gl_VertexID, 2) == 1){
        angle = 2*PI/nb_vertices*(gl_VertexID - 1)-PI/4;
        float inner_radius = radius-radius*fill_ratio;
        relativePos = vec3(inner_radius*cos(angle), inner_radius*sin(angle), 0);
        vec4 centerWorld = model_view_matrix*vec4(point_center, 1);
        vec4 billboard_vertex_pos = centerWorld + vec4(relativePos, 1);
        vpos = proj_matrix*billboard_vertex_pos;
        if (second_pass){
            float outline_width = 2; //px
            vec2 normal = -normalize(relativePos.xy);
            vec2 offset = normal / screen_dims * outline_width * vpos.w;
            vpos.xy += offset;
        }
    }
    else {
        angle = 2*PI/nb_vertices*gl_VertexID-PI/4;
        relativePos = vec3(radius*cos(angle), radius*sin(angle), 0);
        vec4 centerWorld = model_view_matrix*vec4(point_center, 1);
        vec4 billboard_vertex_pos = centerWorld + vec4(relativePos, 1);
        vpos = proj_matrix*billboard_vertex_pos;

        if (second_pass){
            float outline_width = 2; //px
            vec2 normal = normalize(relativePos.xy);
            vec2 offset = normal / screen_dims * outline_width * vpos.w;
            vpos.xy += offset;
        }
    }

    gl_Position = vpos;
}
