#version 130

in vec2 TexCoord;

// Maps U to Red, V to green.
void main() {
    gl_FragColor = vec4(TexCoord.x, TexCoord.y, 1.0f, 1.0f);
}
