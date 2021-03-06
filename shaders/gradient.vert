
#version 420 core
// Input vertex data
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;

// Output
out vec3 color;
out vec3 normal;

// Projection * view * model matrix
uniform mat4 MVP;

void main()
{
	// Red goes   0 - 0 - 1
	// Green goes 0 - 1 - 0
	// Blue goes  1 - 0 - 0
	color = vec3(
		2 * clamp(pos.z - 0.5f, 0, 1),
		1 - 2 * abs(pos.z - 0.5f),
		2 * clamp(0.5f - pos.z, 0, 1));

	// Skip transforming the normal, the model matrix should be translation only anyway
	normal = norm;
	gl_Position = MVP * vec4(pos.xyz, 1);
}
