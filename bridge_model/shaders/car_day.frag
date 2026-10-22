#include "lighting_day.glslh"
#include "fog.glslh"

struct Material
{
	vec3 albedo;
	vec3 specular;
	int shininess;
};
uniform Material materials[5];

in vec3 world_position;
in vec3 position_in_view;
in vec3 delta_position_in_view;
in vec3 aNormal;
in flat int aMaterialIdx;
in flat vec3 carColor;

layout(location = 0) out vec3 fragColor;
layout(location = 2) out vec3 delta_position;

void main()
{
	Material material;
	if(aMaterialIdx < 0)
	{
		material.albedo = carColor;
		material.specular = vec3(0.0);
		material.shininess = 0;
	}
	else
	{
		material = materials[aMaterialIdx];
	}
	vec3 color = material.albedo * sun.ambient;
	float view_dist = length(position_in_view);
	float LdotN = dot(sun.light_dir_and_radius.xyz, aNormal);
	vec3 dpdx = dFdx(world_position);
	vec3 dpdy = dFdy(world_position);
	vec3 surface_normal = normalize(cross(dpdx, dpdy));
	if(LdotN > 1e-5 && dot(surface_normal, sun.light_dir_and_radius.xyz) > 1e-5)
	{
		float specular_factor = 0.0;
		if(material.shininess > 0)
		{
			vec3 halfway = normalize(sun.light_dir_and_radius.xyz + normalize(camera.inv_view[3].xyz - world_position));
			float HdotN = dot(halfway, aNormal);
			if(HdotN > 0)
			{
				specular_factor = pow(HdotN, material.shininess);
			}
		}
		float shadow = shadowPCSS(world_position, dpdx, dpdy, surface_normal, LdotN + specular_factor);
		color += (shadow * LdotN * material.albedo + shadow * specular_factor * material.specular) * sun.diffuse_specular;
	}
	
	float fog_factor = fogFactor(world_position);
	fragColor = fog_factor * color + (1.0 - fog_factor) * sun.sky_color.rgb;
	delta_position = delta_position_in_view;
}
