uniform ivec2 sample_cnt;

layout(binding = 0) uniform sampler2D tex;

in vec2 tex_coord;

out vec3 frag_color;

void main()
{
    vec2 tex_step = vec2(dFdx(tex_coord).x, dFdy(tex_coord).y);
    vec3 color = vec3(0.0);
    for(int i = 0; i < sample_cnt.x; i++)
    {
        for(int j = 0; j < sample_cnt.y; j++)
        {
            color += textureLod(tex, tex_coord +(-0.5 + 1.0 / sample_cnt * vec2(0.5 + i, 0.5 + j)) * tex_step, 0.0).rgb;
        }
    }
    frag_color = 1.0 / (sample_cnt.x * sample_cnt.y) * color;
}
