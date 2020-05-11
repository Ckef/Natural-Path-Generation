
#version 420 core
// Input vertex color
in vec3 color;

// Output fragment color
out vec4 FragColor;

void main()
{
	FragColor = vec4(color, 1);
}
