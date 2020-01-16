#version 130

in vec4 Color;

// Draws fragments a solid color.
void main() {
    gl_FragColor = Color;
}
