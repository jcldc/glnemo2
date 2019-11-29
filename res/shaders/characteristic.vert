#version 130

attribute vec3 position;
uniform mat4 modelviewMatrix;
uniform mat4 projMatrix;
uniform vec3 CameraUp_worldspace;
uniform vec3 CameraRight_worldspace;
uniform vec3 point_center;

void main()
{
    //    gl_Position = projMatrix*modelviewMatrix*vec4(position, 1.0);
    //
    vec3 center =  (modelviewMatrix*vec4(point_center, 1.0f)).xyz;

    vec3 billboard_vertex_pos =
    center
    + CameraRight_worldspace * position.x * vec3(1)
    + CameraUp_worldspace * position.y * vec3(1);

    gl_Position = projMatrix*vec4(billboard_vertex_pos, 1.0f);

}
