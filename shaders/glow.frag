#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform int mode;
uniform vec3 color;
uniform sampler2D haloMap;
uniform vec3 viewPos;
uniform float time;
uniform float fogDensity;

out vec4 frag_color;

void main()
{
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 c;

	if (mode == 0)
	{

		float facing = abs(dot(normalize(Normal), viewDir));
		float along = pow(1.0 - TexCoord.y, 2.0);
		c = color * facing * facing * along;
	}
	else if (mode == 1)
	{
		c = color * texture(haloMap, TexCoord).r;
	}
	else
	{

		float rim = 1.0 - abs(dot(normalize(Normal), viewDir));
		float lines = 0.6 + 0.4 * step(0.5, fract(FragPos.y * 20.0 - time * 1.5));
		c = color * (0.15 + 1.2 * rim * rim) * lines;
	}

	float d = length(viewPos - FragPos);
	c *= exp(-d * fogDensity);
	frag_color = vec4(c, 1.0);
}
