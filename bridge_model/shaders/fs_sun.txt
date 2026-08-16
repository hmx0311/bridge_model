#include "lighting_day_defines.h"

uniform float horizonY;

layout(pixel_center_integer) in vec4 gl_FragCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 brightColor;


void main()
{
	vec3 sun_color = 50.0 * sun.diffuse_specular;
	if(gl_FragCoord.y < horizonY)
	{
		float sunPercentage = gl_FragCoord.y + 1 - horizonY;
		if(sunPercentage < 0)
		{
			discard;
		}
		sun_color *= sunPercentage;
		fragColor = sun_color + (1 - sunPercentage) * sun.sky_color;
	}
	else
	{
		fragColor = sun_color;
	}
	brightColor = sun_color;
}
