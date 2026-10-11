#version 330 core

in vec3 BeamPos;
in vec3 FragPos;
in vec3 Normal;
in vec2 Corner;

uniform int mode;
uniform vec3 color;
uniform vec3 viewPos;
uniform float time;

out vec4 frag_color;

void main()
{

	float along = smoothstep(1.0, 1.15, BeamPos.z) * (1.0 - smoothstep(1.6, 3.2, BeamPos.z));
	vec3 c;

	if (mode == 0)
	{

		vec2 r = BeamPos.xy;
		vec2 edge = smoothstep(0.0, 0.35, r) * smoothstep(0.0, 0.35, 1.0 - r);

		float facing = abs(dot(normalize(Normal), normalize(viewPos - FragPos)));

		float streaks = 0.55 + 0.25 * sin(r.x * 31.0 + sin(r.y * 7.0) * 2.0 + time * 0.3)
							 + 0.20 * sin(r.x * 67.0 - r.y * 13.0);
		c = color * edge.x * edge.y * facing * streaks * along;
	}
	else
	{

		float d = 1.0 - dot(Corner, Corner);
		if (d <= 0.0)
			discard;
		float twinkle = 0.6 + 0.4 * sin(time * 3.0 + BeamPos.x * 50.0);
		c = color * d * twinkle * along;
	}
	frag_color = vec4(c, 1.0);
}
