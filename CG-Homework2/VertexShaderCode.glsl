#version 430
out vec4 fragColor;

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct DirectionalLight {
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct SpotLight {
	vec3 position;
	vec3 direction;

	float cutOff;
	float outerCutOff;
	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in vec3 fragPosition;
in vec3 normal;
in vec3 texCoords;

uniform vec3 directionalLightShininess;
uniform float directionalLightShininess2;
uniform int spotlight;

uniform vec3 viewPosition;
uniform DirectionalLight dirLight;
uniform PointLight pointLights[5];
uniform SpotLight spotLight;
uniform Material material;

uniform sampler2D texture_diffuse0;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_specular0;
uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;

vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 ambient = light.ambient * vec3(texture(texture_diffuse0, texCoords));
	vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse0, texCoords));
	vec3 specular = light.specular * spec * vec3(texture(texture_specular0, texCoords));
	return directionalLightShininess2 * directionalLightShininess * (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	float distance = length(light.position - fragPos);
	float attenuation = 2.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	vec3 ambient = light.ambient * vec3(texture(texture_diffuse0, texCoords));
	vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse0, texCoords));
	vec3 specular = light.specular * spec * vec3(texture(texture_specular0, texCoords));
	return attenuation * (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	vec3 ambient = light.ambient * vec3(texture(texture_diffuse0, texCoords));
	vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse0, texCoords));
	vec3 specular = light.specular * spec * vec3(texture(texture_specular0, texCoords));
	if (spotlight == 1)
		return 4 * attenuation * intensity * (ambient + diffuse + specular);
	else
		return 0 * attenuation * intensity * (ambient + diffuse + specular);
}

void main()
{
	vec3 norm = normalize(normal);
	vec3 viewDir = normalize(viewPosition - fragPosition);

	vec3 result = CalcDirLight(dirLight, norm, viewDir);
	for (int i = 0; i < 5; i++)
	{
		result += CalcPointLight(pointLights[i], norm, fragPosition, viewDir);
	}
	result += CalcPointLight(spotLight, norm, fragPosition, viewDir);

	//fragColor = texture(texture_diffuse0, texCoords);
	fragColor = vec4(result, 1.0);
}
