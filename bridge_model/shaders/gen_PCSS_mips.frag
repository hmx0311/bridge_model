uniform int layer;
uniform int mip_level;

layout(binding = 2) uniform sampler2DArray shadow_tex_depth;

void main() 
{
    ivec3 baseCoord = ivec3(ivec2(gl_FragCoord.xy) * 2, layer);

    float v0 = texelFetch(shadow_tex_depth, baseCoord, mip_level).x;
    float v1 = texelFetch(shadow_tex_depth, baseCoord + ivec3(1, 0, 0), mip_level).x;
    float v2 = texelFetch(shadow_tex_depth, baseCoord + ivec3(0, 1, 0), mip_level).x;
    float v3 = texelFetch(shadow_tex_depth, baseCoord + ivec3(1, 1, 0), mip_level).x;

    float min_depth = min(min(v0, v1), min(v2, v3));
    gl_FragDepth = min_depth;
}