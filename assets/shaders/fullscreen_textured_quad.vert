#version 130

in vec4 Position;
out vec2 TexCoord;

// Passes position through without modification and generates texture
// coordinates for drawing to a quad with the axis in the lower left
// corner.
void main() {
    // Transform position coordinates from [-1, 1] -> [0, 1].
    TexCoord = (Position.xy + 1) / 2.0;
    gl_Position = vec4(Position.x, Position.y, 0.5, 1.0);
}
