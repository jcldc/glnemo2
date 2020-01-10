#version 130
in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 modelviewMatrix; // TODO change name
uniform mat4 projMatrix;
uniform vec3 point_center;
uniform float offset;

void main()
{
    vec3 centerWorld = (modelviewMatrix*vec4(point_center, 1)).xyz;
//    vec3 billboard_vertex_pos = centerWorld + vec3( vertex.xy, 0);

    vec3 billboard_vertex_pos = centerWorld + vec3( vertex.xy, 0) + vec3(offset, offset, 0);

    gl_Position = projMatrix*vec4(billboard_vertex_pos, 1);

    TexCoords = vertex.zw;
}

