
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
	// I like me some yellow terrain :)
	color = vec3(0.8, 0.8, 0);

	// Skip transforming the normal, the model matrix should be translation only anyway
	normal = norm;
	gl_Position = MVP * vec4(pos.xyz, 1);
}
