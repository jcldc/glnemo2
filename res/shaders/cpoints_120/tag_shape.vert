#version 120
#extension GL_EXT_gpu_shader4 : require

uniform mat4 model_view_matrix;
//uniform mat4 model_view_matrixInverse;
uniform mat4 proj_matrix;
//uniform float screen_scale;
uniform bool second_pass;
uniform vec3 color;
uniform vec3 selected_color;

attribute vec3 point_center;
attribute float radius;
varying vec3 vertex_color;

void main()
{
    float rad = radius * 0.65; // factor to place tag just under the cpoint name
    vec2 relativePos;
    if (gl_VertexID == 0){
        gl_Position = proj_matrix*model_view_matrix*vec4(point_center, 1);
        if(second_pass)
            vertex_color = selected_color;
        else
            vertex_color = color;
    }
    else{
        vertex_color = color;
        if (gl_VertexID == 1){
            relativePos = vec2(rad, rad);
        }
        else if (gl_VertexID == 2){
            relativePos = vec2(rad+rad*.5, rad);
        }

//        float w = (proj_matrix*model_view_matrix * vec4(0,0,0,1)).w;
//        w *= screen_scale;

        vec4 centerWorld = model_view_matrix*vec4(point_center, 1.0f);
        vec4 billboard_vertex_pos = centerWorld + vec4(relativePos, 0.f, 0.f);

//        billboard_vertex_pos = model_view_matrixInverse*billboard_vertex_pos;
//        if(screen_scale != -1)
        gl_Position = proj_matrix*billboard_vertex_pos;


//        gl_Position = proj_matrix*billboard_vertex_pos;

    }
}
