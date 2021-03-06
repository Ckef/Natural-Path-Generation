
#version 420 core
// Input vertex data
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec3 col;

// Output
out vec3 color;
out vec3 normal;

// Projection * view * model matrix
uniform mat4 MVP;

void main()
{
	// Skip transforming the normal, the model matrix should be translation only anyway
	normal = norm;
	color = col;
	gl_Position = MVP * vec4(pos.xyz, 1);
}
