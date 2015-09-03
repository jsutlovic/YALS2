#version 330

layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;

uniform mat4 MVP;
out vec4 vertex_color;

void main()
{
	gl_Position = MVP * vec4(position, 0.0, 1.0);
	vertex_color = color;
}
