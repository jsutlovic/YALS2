#version 330

uniform float scale;
in vec4 vert_color;
in vec2 vert_tex_coord;
out vec4 output_color;

uniform sampler2D tex;

void main() {
    vec2 loc = (2.0*vert_tex_coord - 1.0)/(2.0 * scale);
    output_color = texture(tex, vert_tex_coord);
}
