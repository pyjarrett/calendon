#version 130

in vec2 TexCoord;

uniform sampler2D Texture;

void main() {
    gl_FragColor = texture(Texture, TexCoord.xy);
}
