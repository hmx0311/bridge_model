#include "car_attrib_locations.h"
#include "camera_defines.h"

uniform int numLightOnCars;

layout(location = CAR_ATTRIB_VERTEX_LOCATION)			in vec3 vertex;
layout(location = CAR_ATTRIB_NORMAL_LOCATION)			in vec3 normal;
layout(location = CAR_ATTRIB_MATERIAL_INDEX_LOCATION)	in int materialIdx;
layout(location = CAR_ATTRIB_COLOR_LOCATION)			in vec3 color;
layout(location = CAR_ATTRIB_TRANSFORM_LOCATION)		in mat4 modelMat;
layout(location = CAR_ATTRIB_OLD_TRANSFORM_LOCATION)	in mat4 old_transform;

out vec3 world_position;
out vec3 position_in_view;
out vec3 delta_position_in_view;
out vec3 aNormal;
out flat int aMaterialIdx;
out flat vec3 carColor;
out flat int instanceId;

void main()
{
	world_position = (modelMat * vec4(vertex, 1.0)).xyz;
	position_in_view = (camera.view * vec4(world_position, 1.0)).xyz;
	delta_position_in_view = position_in_view - (camera.view * old_transform * vec4(vertex, 1.0)).xyz;
	aNormal = mat3(modelMat) * normal;
	aMaterialIdx = materialIdx;
	instanceId = gl_InstanceID;
	if(gl_InstanceID < numLightOnCars && materialIdx > 2)
	{
		aMaterialIdx += 2;
	}
	carColor = color;
	gl_Position = camera.projection * vec4(position_in_view, 1.0);
	gl_Position.xy += camera.taa_offset * gl_Position.w;
}
