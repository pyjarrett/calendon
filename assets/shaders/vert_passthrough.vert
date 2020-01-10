#version 130

in vec3 Position;
out vec2 TexCoord;

void main() {
    TexCoord = (Position.xy + 1) / 2.0;
    gl_Position = vec4(Position.x, Position.y, 0.5, 1.0);
}

