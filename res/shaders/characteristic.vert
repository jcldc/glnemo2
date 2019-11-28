#version 130

attribute vec3 position;
uniform mat4 modelviewMatrix;
uniform mat4 projMatrix;
uniform vec3 CameraUp_worldspace;
uniform vec3 CameraRight_worldspace;

void main()
{
    gl_Position = projMatrix*modelviewMatrix*vec4(position, 1.0);

//    gl_Position = vec4(
//    position
//    + CameraRight_worldspace * vec3(0.5f, 1.f, 1.0f) * vec3(1)
//    + CameraUp_worldspace * vec3(1.0f, 0.5f, 1.0f) * vec3(1), 1.0f);

}
