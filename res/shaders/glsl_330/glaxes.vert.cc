#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat3 normalMatrix;

out vec3 v_normal;
out vec3 v_viewDir;

void main()
{
    vec4 worldPos = modelMatrix * vec4(position, 1.0);
    vec4 viewPos = viewMatrix * worldPos;

    v_normal = normalize(normalMatrix * normal);
    v_viewDir = normalize(-viewPos.xyz);

    gl_Position = projMatrix * viewPos;
}
