layout(location = 0) in vec3 vertex;
layout(location = 3) in mat4 transform;

void main()
{
	gl_Position = transform * vec4(vertex, 1.0);
}
