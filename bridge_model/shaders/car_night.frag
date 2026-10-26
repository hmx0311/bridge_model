#include "scene_constances.h"
#include "lighting_day_defines.h"
#include "lighting_night.glslh"
#include "fog.glslh"

struct Material
{
	vec3 albedo;
	vec3 specular;
	int shininess;
};
uniform Material materials[7];

in vec3 world_position;
in vec3 delta_position_in_view;
in vec3 aNormal;
in flat int aMaterialIdx;
in flat vec3 carColor;
in flat int instanceId;

layout(location = RENDER_TARGET_COLOR) out vec3 frag_color;
layout(location = RENDER_TARGET_BLOOM) out vec3 frag_bloom;
layout(location = RENDER_TARGET_DELTA_POSITION) out vec3 frag_delta_position;
layout(location = RENDER_TARGET_ALBEDO) out vec3 frag_albedo;

void main()
{
	vec3 view_dir = normalize(camera.inv_view[3].xyz - world_position);
	Material material;
	if(aMaterialIdx < 0)
	{
		material.albedo = carColor;
		material.shininess = 0;
	}
	else
	{
		material = materials[aMaterialIdx];
	}
	frag_albedo = material.albedo;
	vec3 color = material.albedo;
	float maxColor = max(max(color.r, color.g), color.b);
	if(maxColor <= 1.0)
	{
		vec3 lighting = sun.ambient;
		int lightingOffset = instanceId * LIGHTING_SIZE_PER_CAR;
		int end = lightingOffset + 1 + car_lighting.light_indices[lightingOffset];
		for(int i = lightingOffset + 1; i < end; i++)
		{
			int light_idx = car_lighting.light_indices[i];
			vec3 light_dir = world_position - vec3(tile_lights[light_idx].position);
			float light_distance = length(light_dir);
			light_dir /= light_distance;
			float intensity = -dot(light_dir, aNormal) / (light_distance * light_distance);
			if(intensity > 0.0)
			{
				if(material.shininess > 0.0)
				{
					vec3 halfway = normalize(-light_dir + view_dir);
					float HdotN = dot(halfway, aNormal);
					if(HdotN > 0)
					{
						intensity += pow(HdotN, material.shininess);
					}
				}
				intensity *= lightCoverage(world_position, light_idx);
				lighting += intensity * CAR_LIGHT_INTENSITY;
			}
		}
		color *= lighting;
	}

	float fog_factor = fogFactor(world_position);
	color = fog_factor * color + (1.0 - fog_factor) * sun.sky_color.rgb;
	frag_color = color;
	frag_bloom = maxColor > 1 ? color : vec3(0.0);
	frag_delta_position = delta_position_in_view;
}
