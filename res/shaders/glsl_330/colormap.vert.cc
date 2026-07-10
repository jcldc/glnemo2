#version 330 core

// Vertex position (2D screen space, in pixels).
layout(location = 0) in vec2 aPos;

// UV coordinate along the colormap axis (0.0 → 1.0).
// The CPU sets u=0 at the left/bottom edge and u=1 at the right/top edge.
layout(location = 1) in float aTexU;

out float vTexU;

uniform mat4 uProjection;   // orthographic pixel → NDC

void main()
{
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
    vTexU = aTexU;
}
