#version 130

in vec2 Position2;
in vec2 TexCoord2;
out vec2 TexCoord;
uniform mat4 ViewModel;
uniform mat4 Projection;

void main() {
    // Assumes sprite coordinates are [0,0] lower left corner to [1,1] upper right.
    TexCoord = TexCoord2;
    gl_Position = Projection * ViewModel * vec4(Position2.x, Position2.y, 0.5, 1.0);
}
