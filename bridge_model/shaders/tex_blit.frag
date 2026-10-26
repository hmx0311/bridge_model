layout(binding = 0) uniform sampler2D tex;

in vec2 texCoord;

out vec3 frag_color;

void main()
{
    frag_color = texture(tex, texCoord).rgb;
}
