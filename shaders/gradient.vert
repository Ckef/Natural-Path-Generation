
#version 420 core
// Input vertex data
layout (location = 0) in vec3 pos;

// Output vertex color
out vec4 color;

// Projection * view * model matrix
uniform mat4 MVP;

void main()
{
	// Red goes   0 - 0 - 1
	// Green goes 0 - 1 - 0
	// Blue goes  1 - 0 - 0
	color = vec4(
		2 * clamp(pos.z - 0.5f, 0, 1),
		1 - 2 * abs(pos.z - 0.5f),
		2 * clamp(0.5f - pos.z, 0, 1),
		1);

	gl_Position = MVP * vec4(pos.xy, pos.z*20, 1);
}
