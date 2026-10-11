#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int mode;
uniform float time;

out vec3 BeamPos;
out vec3 FragPos;
out vec3 Normal;
out vec2 Corner;

void main()
{
	vec4 worldPos;
	if (mode == 0)
	{
		BeamPos = vec3(pos.xy / pos.z, pos.z);
		worldPos = model * vec4(pos, 1.0);
		Normal = mat3(transpose(inverse(model))) * normal;
		Corner = vec2(0.0);
	}
	else
	{

		float phase = normal.z * 40.0;
		vec3 p = pos;
		p.x = fract(p.x + 0.010 * time + 0.03 * sin(time * 0.5 + phase));
		p.y = fract(p.y - 0.015 * time + 0.02 * sin(time * 0.7 + phase * 1.3));
		BeamPos = p;
		vec4 center = model * vec4(p.z * vec3(p.xy, 1.0), 1.0);

		Corner = sign(normal.xy);
		vec3 right = vec3(view[0][0], view[1][0], view[2][0]);
		vec3 up = vec3(view[0][1], view[1][1], view[2][1]);
		worldPos = vec4(center.xyz + (right * Corner.x + up * Corner.y) * 0.008, 1.0);
		Normal = vec3(0.0, 0.0, 1.0);
	}

	FragPos = vec3(worldPos);
	gl_Position = projection * view * worldPos;
}
