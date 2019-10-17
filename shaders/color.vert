
#version 420 core
// Input vertex data
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 col;

// Output vertex color
out vec4 color;

// Projection * view * model matrix
uniform mat4 MVP;

void main()
{
	color = vec4(col, 1);
	gl_Position = MVP * vec4(pos.xyz, 1);
}
