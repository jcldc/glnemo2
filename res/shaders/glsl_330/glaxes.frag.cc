#version 330 core

in vec3 v_normal;
in vec3 v_viewDir;

uniform vec3 axisColor;

out vec4 fragColor;

void main()
{
    vec3 N = normalize(v_normal);
    vec3 V = normalize(v_viewDir);

    // Simple headlight shading: light comes from the camera direction
    float diff = max(dot(N, V), 0.0);
    float ambient = 0.35;

    vec3 finalColor = axisColor * (ambient + (1.0 - ambient) * diff);
    fragColor = vec4(finalColor, 1.0);
}
