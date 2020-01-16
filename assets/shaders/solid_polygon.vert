#version 130

uniform mat4 Projection;
uniform mat4 ViewModel;
uniform vec4 PolygonColor;

in vec4 Position;
out vec4 Color;

// Passes position through without modification and generates texture
// coordinates for drawing to a quad with the axis in the lower left
// corner.
void main() {
    gl_Position = Projection * vec4(Position.x, Position.y, Position.z, 1.0);
    Color = PolygonColor;
}
