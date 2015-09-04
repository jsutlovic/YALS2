#version 330

layout(location = 0) in vec2 position;

uniform vec4 colors[4];
uniform mat4 MVP;
out vec4 vert_color;

void main()
{
	gl_Position = MVP * vec4(position, 0.0, 1.0);
	vert_color = colors[0];
}
