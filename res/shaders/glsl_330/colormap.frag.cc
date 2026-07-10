#version 330 core

in  float vTexU;
out vec4  FragColor;

// 1D RGB texture built from the R, G, B vectors on the CPU.
// Sampled with GL_LINEAR for smooth color transitions.
uniform sampler1D uColormap;

// Global opacity (1.0 = fully opaque).
uniform float uAlpha;

void main()
{
    vec3 color = texture(uColormap, vTexU).rgb;
    FragColor  = vec4(color, uAlpha);
}
