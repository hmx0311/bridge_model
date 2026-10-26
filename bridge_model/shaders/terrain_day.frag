#include "lighting_day.glslh"
#include "fog.glslh"

layout(binding = 0) uniform sampler2D albedo_tex;

in vec3 world_position;
in vec3 position_in_view;
in vec3 aNormal;
in vec2 aTexCoord;

layout(location = RENDER_TARGET_COLOR) out vec3 frag_color;
layout(location = RENDER_TARGET_DELTA_POSITION) out vec3 frag_delta_position;
layout(location = RENDER_TARGET_ALBEDO) out vec3 frag_albedo;

void main()
{
	vec3 color = mix(texture(albedo_tex, aTexCoord).rgb, vec3(0.5, 0.5, 0.3), smoothstep(-0.5, -3.0, world_position.z));
	frag_albedo = color;
	vec3 lighting = sun.ambient;
	vec3 normal = normalize(aNormal);
	float LdotN = dot(sun.light_dir_and_radius.xyz, normal);
	vec3 dpdx = dFdx(world_position);
	vec3 dpdy = dFdy(world_position);
	vec3 surface_normal = normalize(cross(dpdx, dpdy));
	if(LdotN > 1e-5)
	{
		vec3 position = world_position;
		if(dot(surface_normal, sun.light_dir_and_radius.xyz) < 1e-5)
		{
			position += normal * 0.05 * length(position_in_view);
			surface_normal = normal;
		}
		float shadow = shadowPCSS(world_position, dpdx, dpdy, surface_normal, LdotN);
		lighting += shadow * LdotN * sun.diffuse_specular;
	}
	color *= lighting;
	float fog_factor = fogFactor(world_position);
	frag_color = fog_factor * color + (1.0 - fog_factor) * sun.sky_color.rgb;
	frag_delta_position = vec3(0.0);
}
