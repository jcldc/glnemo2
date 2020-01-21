#version 140

uniform int nb_vertices;
uniform mat4 modelviewMatrix; // TODO change name
uniform mat4 projMatrix;
uniform bool is_filled;
uniform int subdivisions;
uniform bool second_pass;
uniform ivec2 screen_dims;

in vec3 point_center;
in float radius;
in float is_selected;

flat out float selected;

float PI = 3.14159265359;

void main()
{
    selected = is_selected;
    vec3 spherePos;
    int i,j;
    float x_angle, y_angle;
    vec2 circle;

    if(gl_VertexID < subdivisions*subdivisions/2+1){

        i = gl_VertexID % subdivisions;
        j = gl_VertexID / subdivisions;

        x_angle = i*2*PI/subdivisions;
        y_angle = j*2*PI/subdivisions;

        circle = vec2(cos(x_angle), sin(x_angle));

        mat3 rot = mat3(1, 0, 0,
                        0, cos(y_angle), sin(y_angle),
                        0,  -sin(y_angle),cos(y_angle));

        spherePos = vec3(rot*vec3(circle, 0))*radius;
    }
    else{
        i = gl_VertexID % subdivisions;
        j = gl_VertexID / (subdivisions+1);
        i -= j;
        x_angle = i*2*PI/subdivisions;
        y_angle = j*2*PI/subdivisions;

        circle = vec2(cos(x_angle), sin(x_angle));
        float c_radius = sin(y_angle);
        spherePos = vec3(-cos(y_angle), circle*c_radius)*radius;
    }

    vec3 vertex_pos = point_center+spherePos;
    vec4 vpos = projMatrix * modelviewMatrix*vec4(vertex_pos,1);


    if (second_pass){
        float outline_width = 5; //px
        vec2 normal = normalize((projMatrix*modelviewMatrix*vec4(spherePos, 1)).xy);
        vec2 offset = normal / screen_dims * outline_width * vpos.w;

        vpos.xy += offset;
    }

    gl_Position = vpos;
}
