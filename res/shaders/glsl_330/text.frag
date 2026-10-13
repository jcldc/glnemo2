#version 330 core
in  vec2 vTexCoord;
out vec4 FragColor;
uniform sampler2D uGlyphTexture;
uniform vec4      uTextColor;
void main() {
    float alpha = texture(uGlyphTexture, vTexCoord).r;
    FragColor   = vec4(uTextColor.rgb, uTextColor.a * alpha);
}

