
#version 420 core
// Input vertex data
layout (location = 0) in vec3 pos;

// Projection * view * model matrix
uniform mat4 MVP;

void main()
{
	gl_Position = MVP * vec4(pos, 1);
}
