#version 330

layout(location = 0) in vec2 position;

uniform usamplerBuffer world_texture_buffer;
uniform vec4 colors[5];
uniform mat4 MVP;
uniform uint inv_state;
out vec4 vert_color;
int cell_id, shift, data_offset;
int cell_val;

void main()
{
    gl_Position = MVP * vec4(position, 0.0, 1.0);
    cell_id = gl_VertexID / 6;
    data_offset = cell_id >> 4;
    shift = cell_id & 0xf;
    cell_val = int(texelFetch(world_texture_buffer, data_offset).r);
    cell_val = (cell_val >> (shift*2)) & (3 << inv_state) & 3;
    vert_color = colors[cell_val << inv_state];
}
