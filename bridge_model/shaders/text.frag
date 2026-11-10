#include "text_altas_constances.h"

uniform vec2 tex_offset;

layout(binding = 0) uniform sampler2D tex;

in vec2 tex_coord;

out vec4 frag_color;

void main()
{
    frag_color = vec4(1.0, 1.0, 1.0, texture(tex, vec2(tex_coord.x / TEXT_ALTAS_CNT_X, (1.0 - tex_coord.y) / TEXT_ALTAS_CNT_Y) + tex_offset).r);
}
