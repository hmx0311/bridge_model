#include "lighting_day.glslh"
#include "fog.glslh"

struct Material
{
	vec3 ambientAndDiffuse;
	vec3 specular;
	int shininess;
};
uniform Material materials[5];

in vec3 modelPos;
in vec3 modelPosView;
in vec3 viewRay;
in vec3 aNormal;
in flat int aMaterialIdx;
in flat vec3 carColor;

out vec3 fragColor;

void main()
{
	Material material;
	if(aMaterialIdx < 0)
	{
		material.ambientAndDiffuse = carColor;
		material.specular = vec3(0.0);
		material.shininess = 0;
	}
	else
	{
		material = materials[aMaterialIdx];
	}
	vec3 ambientAndDiffuseLighting = sun.ambient;
	vec3 specularLighting = vec3(0.0);
	float view_dist = length(modelPosView);
	float LdotN = dot(sun.light_dir, aNormal);
	vec3 dpdx = dFdx(modelPos);
	vec3 dpdy = dFdy(modelPos);
	vec3 surface_normal = normalize(cross(dpdx, dpdy));
	if(LdotN > 1e-5 && dot(surface_normal, sun.light_dir) > 1e-5)
	{
		float shadow = shadowPCSS(modelPos, dpdx, dpdy, surface_normal, LdotN);
		ambientAndDiffuseLighting += shadow * LdotN * sun.diffuse_specular;
		if(material.shininess > 0)
		{
			vec3 halfway = normalize(sun.light_dir + normalize(viewRay));
			float HdotN = dot(halfway, aNormal);
			if(HdotN > 0)
			{
				specularLighting = shadow * pow(HdotN, material.shininess) * sun.diffuse_specular;
			}
		}
	}
	vec3 color = ambientAndDiffuseLighting * material.ambientAndDiffuse + specularLighting * material.specular;
	
	float fog_factor = fogFactor(length(viewRay));
	fragColor = fog_factor * color + (1.0 - fog_factor) * sun.sky_color;
}
