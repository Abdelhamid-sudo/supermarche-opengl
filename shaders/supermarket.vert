#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform int worldUV;
uniform float uvScale;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

void main()
{
	FragPos = vec3(model * vec4(pos, 1.0f));
	Normal = mat3(transpose(inverse(model))) * normal;

	TexCoord = texCoord;
	if (worldUV == 1)
	{

		vec3 n = abs(normalize(Normal));
		if (n.x > n.y && n.x > n.z)
			TexCoord = FragPos.zy * uvScale;
		else if (n.z > n.y)
			TexCoord = FragPos.xy * uvScale;
		else
			TexCoord = FragPos.xz * uvScale;
	}

	gl_Position = projection * view * model * vec4(pos, 1.0f);
}
