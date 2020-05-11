
#version 420 core
// Input vertex data
in vec3 normal;
in vec3 color;

// Output
out vec4 FragColor;

// Some lighting constants (maybe make m modifiable...)
const vec3 LightDirection  = vec3(0, 2, -1);
const float AmbientLight   = 0.2;

void main()
{
	float diff = dot(normal, normalize(-LightDirection));
	FragColor = vec4((diff + AmbientLight) * color, 1);
}
