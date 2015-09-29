#version 330

in vec2 vert_tex_coord;
out vec4 output_color;

uniform sampler2D tex;

void main() {
    output_color = texture(tex, vert_tex_coord);
}
