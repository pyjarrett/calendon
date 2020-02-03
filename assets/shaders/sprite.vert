#version 130

in vec4 Position;
out vec2 TexCoord;
uniform mat4 ViewModel;
uniform mat4 Projection;

void main() {
    TexCoord = (Position.xy + 1) / 2.0;
    gl_Position = Projection * ViewModel * vec4(Position.x, Position.y, 0.5, 1.0);
}
