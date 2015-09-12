#version 330

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;
uniform vec4 color;
uniform mat4 MVP;
out vec4 vert_color;
out vec2 vert_tex_coord;

void main()
{
    gl_Position = MVP * vec4(position, 0.0, 1.0);
    vert_color = color;
    vert_tex_coord = texCoord;
}
