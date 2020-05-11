
#version 420 core
// Input vertex data
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 col;

// Output vertex color
out vec3 color;

// Projection * view * model matrix
uniform mat4 MVP;

void main()
{
	color = col;
	gl_Position = MVP * vec4(pos.xyz, 1);
}
