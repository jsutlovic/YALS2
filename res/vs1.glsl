#version 330

layout(location = 0) in vec2 position;

uniform usamplerBuffer world_texture_buffer;
uniform vec4 colors[4];
uniform mat4 MVP;
out vec4 vert_color;
int cell_id, shift, data_offset;
int cell_val;

void main()
{
    gl_Position = MVP * vec4(position, 0.0, 1.0);
    cell_id = gl_VertexID / 6;
    data_offset = cell_id / 16;
    shift = 15 - (cell_id % 16);
    cell_val = int(texelFetch(world_texture_buffer, data_offset).r >> (shift*2)) & 3;
    vert_color = colors[cell_val];
}
