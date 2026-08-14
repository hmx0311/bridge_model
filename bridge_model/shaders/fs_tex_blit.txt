layout(binding = 0) uniform sampler2D tex;

in vec2 texCoord;

out vec3 fragColor;

void main()
{
    fragColor = texture(tex, texCoord).rgb;
}
