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

layout(binding = 1) uniform sampler2DArrayShadow shadowTex;

in vec3 modelPos;
in vec3 viewRay;
in vec3 aNormal;
in flat int aMaterialIdx;
in flat vec3 carColor;
in flat int instanceId;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 bloomColor;


void main()
{
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
	vec3 color = material.albedo;
	float maxColor = max(max(color.r, color.g), color.b);
	if(maxColor <= 1.0)
	{
		vec3 lighting = sun.ambient;
		int lightingOffset = instanceId * MAX_LIGHT_PER_CAR;
		int end = lightingOffset + 1 + car_lighting.light_indices[lightingOffset];
		for(int i = lightingOffset + 1; i < end; i++)
		{
			int idx = car_lighting.light_indices[i];
			vec3 modelToLight = modelPos - vec3(car_light.positions[idx]);
			float lightDistance = length(modelToLight);
			modelToLight /= lightDistance;
			float intensity = -dot(modelToLight, aNormal) / (lightDistance * lightDistance);
			vec4 modelPosLight = car_light_shadow.view_proj[idx] * vec4(modelPos, 1.0);
			modelPosLight.xy /= modelPosLight.w;
			float r2 = dot(modelPosLight.xy, modelPosLight.xy);
			if(lightDistance > 0.45 && intensity > 0 && -modelPosLight.w < modelPosLight.z && modelPosLight.z < modelPosLight.w && r2 < 0.99)
			{
				if(material.shininess > 0)
				{
					vec3 halfway = normalize(-modelToLight + normalize(viewRay));
					float HdotN = dot(halfway, aNormal);
					if(HdotN > 0)
					{
						intensity += pow(HdotN, material.shininess);
					}
				}
				intensity *= lightSmooth(r2);
				int layer = findMSB((idx >> 3) * 3 + 1) >> 1;
				if(layer <= NUM_TILE_LIGHT_SHADOW_LAYERS)
				{
					ivec2 cnt = ivec2(2 << layer, 4 << layer);
					idx -= ((8 << (2 * layer)) - 8) / 3 ;
					vec2 shadowMapPos = vec2(idx % cnt.x, idx / cnt.x) / cnt;
					modelPosLight.xy = (modelPosLight.xy + 1.0) / (2 * cnt) + shadowMapPos;
					modelPosLight.w = 0.5 * (modelPosLight.z / modelPosLight.w + 1.0);
					modelPosLight.z = layer;
					intensity *= texture(shadowTex, modelPosLight).x;
				}
				lighting += intensity * CAR_LIGHT_INTENSITY;
			}
		}
		color *= lighting;
	}

	float fog_factor = fogFactor(length(viewRay));
	color = fog_factor * color + (1.0 - fog_factor) * sun.sky_color;
	fragColor = color;
	bloomColor = maxColor > 1 ? color : vec3(0.0);
}
