#version 130

uniform mat4 modelviewMatrix;
//uniform mat4 modelviewMatrixInverse;
uniform mat4 projMatrix;
//uniform float screen_scale;
uniform bool second_pass;

in vec3 point_center;
in float radius;

void main()
{
    vec2 relativePos;
    if (gl_VertexID == 0){
        gl_Position = projMatrix*modelviewMatrix*vec4(point_center, 1);
    }
    else{
        if (gl_VertexID == 1){
            relativePos = vec2(radius, radius);
        }
        else if (gl_VertexID == 2){
            relativePos = vec2(radius+radius*.5, radius);
        }

//        float w = (projMatrix*modelviewMatrix * vec4(0,0,0,1)).w;
//        w *= screen_scale;

        vec4 centerWorld = modelviewMatrix*vec4(point_center, 1.0f);
        vec4 billboard_vertex_pos = centerWorld + vec4(relativePos, 0.f, 0.f);

//        billboard_vertex_pos = modelviewMatrixInverse*billboard_vertex_pos;
//        if(screen_scale != -1)
        gl_Position = projMatrix*billboard_vertex_pos;


//        gl_Position = projMatrix*billboard_vertex_pos;

    }
}
