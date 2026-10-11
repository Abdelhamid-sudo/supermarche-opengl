#version 330 core

struct Material
{
	vec3 ambient;
	sampler2D diffuseMap;
	vec3 specular;
	float shininess;
	vec3 color;
	vec3 emission;
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

#define MAX_POINT_LIGHTS 6

uniform DirectionalLight sunLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material material;
uniform vec3 viewPos;

uniform float reserveZ;
uniform vec3 reserveAmbient;
uniform SpotLight doorLight;
uniform PointLight tubeLight;

uniform sampler2DShadow sunShadowMap;
uniform mat4 sunLightSpace;
uniform sampler2DShadow doorShadowMap;
uniform mat4 doorLightSpace;
uniform sampler2DShadow flashShadowMap;
uniform mat4 flashLightSpace;
uniform int flashShadows;

out vec4 frag_color;

vec3 calcDirectionalLightColor(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 albedo);
vec3 calcPointLightColor(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo);
vec3 calcSpotLightColor(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo);
float spotCone(SpotLight light, vec3 fragPos);
float calcShadow(sampler2DShadow shadowMap, mat4 lightSpace, vec3 normal, float offset);

void main()
{
	vec3 albedo = vec3(texture(material.diffuseMap, TexCoord)) * material.color;

	vec3 normal = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragPos);

	vec3 outColor;
	if (FragPos.z > reserveZ)
	{

		float hemi = 0.75 + 0.25 * normal.y;
		outColor = sunLight.ambient * hemi * material.ambient * albedo;

		outColor += calcShadow(sunShadowMap, sunLightSpace, normal, 0.03)
				  * calcDirectionalLightColor(sunLight, normal, viewDir, albedo);

		for (int i = 0; i < MAX_POINT_LIGHTS; i++)
			outColor += calcPointLightColor(pointLights[i], normal, FragPos, viewDir, albedo);
	}
	else
	{

		outColor = reserveAmbient * material.ambient * albedo;
		outColor += calcPointLightColor(tubeLight, normal, FragPos, viewDir, albedo);

		if (spotCone(doorLight, FragPos) > 0.0)
			outColor += calcShadow(doorShadowMap, doorLightSpace, normal, 0.02)
					  * calcSpotLightColor(doorLight, normal, FragPos, viewDir, albedo);
	}

	if (spotLight.on == 1 && spotCone(spotLight, FragPos) > 0.0)
	{
		float shadow = (flashShadows == 1) ? calcShadow(flashShadowMap, flashLightSpace, normal, 0.015) : 1.0;
		outColor += shadow * calcSpotLightColor(spotLight, normal, FragPos, viewDir, albedo);
	}

	outColor += material.emission * vec3(texture(material.diffuseMap, TexCoord));

	frag_color = vec4(outColor, 1.0f);
}

vec3 calcDirectionalLightColor(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 albedo)
{
	vec3 lightDir = normalize(-light.direction);

	float NdotL = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * NdotL * albedo;

	vec3 halfDir = normalize(lightDir + viewDir);
	float NDotH = max(dot(normal, halfDir), 0.0f);
	vec3 specular = light.specular * material.specular * pow(NDotH, material.shininess);

	return (diffuse + specular);
}

vec3 calcPointLightColor(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo)
{
	vec3 lightDir = normalize(light.position - fragPos);

	float NdotL = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * NdotL * albedo;

	vec3 halfDir = normalize(lightDir + viewDir);
	float NDotH = max(dot(normal, halfDir), 0.0f);
	vec3 specular = light.specular * material.specular * pow(NDotH, material.shininess);

	float d = length(light.position - fragPos);
	float attenuation = 1.0f / (light.constant + light.linear * d + light.exponent * (d * d));

	return (diffuse + specular) * attenuation;
}

float spotCone(SpotLight light, vec3 fragPos)
{
	float cosDir = dot(normalize(fragPos - light.position), normalize(light.direction));
	return smoothstep(light.cosOuterCone, light.cosInnerCone, cosDir);
}

vec3 calcSpotLightColor(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo)
{
	vec3 lightDir = normalize(light.position - fragPos);
	vec3 spotDir  = normalize(light.direction);

	float cosDir = dot(-lightDir, spotDir);
	float spotIntensity = smoothstep(light.cosOuterCone, light.cosInnerCone, cosDir);

	spotIntensity *= 0.6 + 0.4 * smoothstep(light.cosInnerCone, mix(light.cosInnerCone, 1.0, 0.7), cosDir);

	float NdotL = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * NdotL * albedo;

	vec3 halfDir = normalize(lightDir + viewDir);
	float NDotH = max(dot(normal, halfDir), 0.0f);
	vec3 specular = light.specular * material.specular * pow(NDotH, material.shininess);

	float d = length(light.position - fragPos);
	float attenuation = 1.0f / (light.constant + light.linear * d + light.exponent * (d * d));

	return (diffuse + specular) * attenuation * spotIntensity;
}

float calcShadow(sampler2DShadow shadowMap, mat4 lightSpace, vec3 normal, float offset)
{
	vec4 p = lightSpace * vec4(FragPos + normal * offset, 1.0);
	vec3 c = p.xyz / p.w * 0.5 + 0.5;
	if (p.w <= 0.0 || c.z >= 1.0)
		return 1.0;

	vec2 texel = 1.0 / vec2(textureSize(shadowMap, 0));
	float lit = texture(shadowMap, vec3(c.xy + vec2(-0.5, -0.5) * texel, c.z))
			  + texture(shadowMap, vec3(c.xy + vec2( 0.5, -0.5) * texel, c.z))
			  + texture(shadowMap, vec3(c.xy + vec2(-0.5,  0.5) * texel, c.z))
			  + texture(shadowMap, vec3(c.xy + vec2( 0.5,  0.5) * texel, c.z));
	return lit * 0.25;
}
