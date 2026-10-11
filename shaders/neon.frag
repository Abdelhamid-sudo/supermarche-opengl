#version 330 core

struct Material
{
	vec3 ambient;
	sampler2D diffuseMap;
	sampler2D specularMap;
	vec3 specular;
	float shininess;
	vec3 emission;
	float wet;
};

struct DirectionalLight
{
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight
{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float exponent;
};

struct SpotLight
{
	vec3 position;
	vec3 direction;
	float cosInnerCone;
	float cosOuterCone;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	int on;

	float constant;
	float linear;
	float exponent;
};

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

#define MAX_POINT_LIGHTS 4
#define MAX_SPOT_LIGHTS 2

uniform DirectionalLight moonLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];
uniform Material material;
uniform vec3 viewPos;

uniform vec3 fogColor;
uniform float fogDensity;

out vec4 frag_color;

vec3 calcDirectionalLightColor(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 albedo, float specMask);
vec3 calcPointLightColor(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float specMask);
vec3 calcSpotLightColor(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float specMask);

void main()
{
	vec4 texel = texture(material.diffuseMap, TexCoord);
	vec3 albedo = texel.rgb;
	float specMask = texture(material.specularMap, TexCoord).r;

	vec3 normal = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragPos);

	vec3 outColor = moonLight.ambient * material.ambient * albedo;

	outColor += calcDirectionalLightColor(moonLight, normal, viewDir, albedo, specMask);

	for (int i = 0; i < MAX_POINT_LIGHTS; i++)
		outColor += calcPointLightColor(pointLights[i], normal, FragPos, viewDir, albedo, specMask);

	for (int i = 0; i < MAX_SPOT_LIGHTS; i++)
		if (spotLights[i].on == 1)
			outColor += calcSpotLightColor(spotLights[i], normal, FragPos, viewDir, albedo, specMask);

	outColor += material.emission * albedo * texel.a;

	float d = length(viewPos - FragPos);
	float density = fogDensity * (1.0 + 2.0 * exp(-0.25 * max(FragPos.y, 0.0)));
	float fog = 1.0 - exp(-d * density);
	outColor = mix(outColor, fogColor, fog);

	frag_color = vec4(outColor, 1.0f);
}

vec3 calcDirectionalLightColor(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 albedo, float specMask)
{
	vec3 lightDir = normalize(-light.direction);

	float NdotL = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * NdotL * albedo;

	vec3 halfDir = normalize(lightDir + viewDir);
	float NDotH = max(dot(normal, halfDir), 0.0f);
	vec3 specular = light.specular * material.specular * specMask * pow(NDotH, material.shininess);

	return (diffuse + specular);
}

vec3 calcPointLightColor(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float specMask)
{
	vec3 lightDir = normalize(light.position - fragPos);

	float NdotL = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * NdotL * albedo;

	vec3 halfDir = normalize(lightDir + viewDir);
	float NDotH = max(dot(normal, halfDir), 0.0f);
	vec3 specular = light.specular * material.specular * specMask * pow(NDotH, material.shininess);

	float d = length(light.position - fragPos);
	float attenuation = 1.0f / (light.constant + light.linear * d + light.exponent * (d * d));

	vec3 wet = vec3(0.0);
	if (material.wet > 0.0)
	{
		float RdotV = max(dot(reflect(-lightDir, normal), viewDir), 0.0);
		wet = light.diffuse * material.wet * (0.3 + specMask) * pow(RdotV, 24.0) * 6.0;
	}

	return (diffuse + specular + wet) * attenuation;
}

vec3 calcSpotLightColor(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float specMask)
{
	vec3 lightDir = normalize(light.position - fragPos);
	vec3 spotDir = normalize(light.direction);

	float cosDir = dot(-lightDir, spotDir);
	float spotIntensity = smoothstep(light.cosOuterCone, light.cosInnerCone, cosDir);

	float NdotL = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * NdotL * albedo;

	vec3 halfDir = normalize(lightDir + viewDir);
	float NDotH = max(dot(normal, halfDir), 0.0f);
	vec3 specular = light.specular * material.specular * specMask * pow(NDotH, material.shininess);

	float d = length(light.position - fragPos);
	float attenuation = 1.0f / (light.constant + light.linear * d + light.exponent * (d * d));

	return (diffuse + specular) * attenuation * spotIntensity;
}
