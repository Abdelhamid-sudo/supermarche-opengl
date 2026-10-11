#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int mode;
uniform float haloSize;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

void main()
{
	vec4 worldPos;
	if (mode == 1)
	{

		vec3 center = vec3(model[3]);
		vec3 right = vec3(view[0][0], view[1][0], view[2][0]);
		vec3 up = vec3(view[0][1], view[1][1], view[2][1]);
		worldPos = vec4(center + (right * pos.x + up * pos.y) * haloSize, 1.0);
	}
	else
		worldPos = model * vec4(pos, 1.0);

	FragPos = vec3(worldPos);
	Normal = mat3(model) * normal;
	TexCoord = texCoord;
	gl_Position = projection * view * worldPos;
}
