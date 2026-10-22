#include "lighting_day_defines.h"

uniform float horizonY;

layout(pixel_center_integer) in vec4 gl_FragCoord;

in vec3 a_delta_position;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 brightColor;
layout(location = 2) out vec3 delta_position;

void main()
{
	vec3 sun_color = 50.0 * sun.diffuse_specular;
	if(gl_FragCoord.y < horizonY)
	{
		float percent = gl_FragCoord.y + 1 - horizonY;
		if(percent < 0)
		{
			discard;
		}
		sun_color *= percent;
		fragColor = sun_color + (1 - percent) * sun.sky_color.rgb;
	}
	else
	{
		fragColor = sun_color;
	}
	brightColor = sun_color;
	delta_position = a_delta_position;
}
