#include "lighting_day.glslh"
#include "fog.glslh"

layout(binding = 0) uniform sampler2D albedo;

in vec3 modelPos;
in vec3 modelPosView;
in vec3 aNormal;
in vec2 aTexCoord;

out vec3 fragColor;

void main()
{
	vec3 color = texture(albedo, aTexCoord).rgb;
	vec3 lighting = sun.ambient;
	vec3 normal = normalize(aNormal);
	float LdotN = dot(sun.light_dir_and_radius.xyz, normal);
	vec3 dpdx = dFdx(modelPos);
	vec3 dpdy = dFdy(modelPos);
	vec3 surface_normal = normalize(cross(dpdx, dpdy));
	if(LdotN > 1e-5 && dot(surface_normal, sun.light_dir_and_radius.xyz) > 1e-5)
	{
		float shadow = shadowPCSS(modelPos, dpdx, dpdy, surface_normal, LdotN);
		lighting += shadow * LdotN * sun.diffuse_specular;
	}
	color *= lighting;
	float fog_factor = fogFactor(length(modelPosView));
	fragColor = fog_factor * color + (1.0 - fog_factor) * sun.sky_color;
}
