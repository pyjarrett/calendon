#version 130

in vec2 TexCoord;

//
// Shades according to screen space X, Y values.
//
void main() {
    gl_FragColor = vec4(TexCoord.x, TexCoord.y, 1.0f, 1.0f);
}

