#version 130

in vec4 Position;
out vec2 TexCoord;
uniform mat4 ViewModel;
uniform mat4 Projection;

void main() {
    // Assumes sprite coordinates are [0,0] lower left corner to [1,1] upper right.
    TexCoord = Position.xy;
    gl_Position = Projection * ViewModel * vec4(Position.x, Position.y, 0.5, 1.0);
}
