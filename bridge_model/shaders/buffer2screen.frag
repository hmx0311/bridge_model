layout(binding = 0) uniform sampler2D colorBuffer;
layout(binding = 1) uniform sampler2D bloomBuffer;

in vec2 texCoord;

out vec3 fragColor;

void main()
{
    ivec2 tex_idx = ivec2(gl_FragCoord);
    vec3 color = texelFetch(colorBuffer, tex_idx, 0).rgb + texture(bloomBuffer, texCoord).rgb;
    fragColor = pow(1 - 1 / (1 + color + color * color), vec3(1 / 2.2));
}
