#version 330 core
 out vec4 FragColor;

 in vec2 TexCoords;


uniform sampler2D gPositionDepth;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D ssao;

struct Light{
	vec3 Position;
	vec3 Color;

	float Linear;
    float Quadratic;
	float Radius;

};
uniform Light light;

void main(){
	vec3 FragPos = texture(gPositionDepth,TexCoords).rgb;
	vec3 Normal = texture(gNormal,TexCoords).rgb;
	vec3 Diffuse = texture(gAlbedo,TexCoords).rgb;
	float AmbientOcclusion = texture(ssao,TexCoords).r;

	vec3 ambient = vec3(0.4*AmbientOcclusion);
	vec3 lighting = ambient;
	vec3 viewDir = normalize(-FragPos);

	vec3 lightDir = normalize(light.Position - FragPos);
	vec3 diffuse = max(dot(Normal,lightDir),0)*Diffuse*light.Color;
	float distance = length(light.Position - FragPos);
	float attenuation = 1.0/(1.0 + light.Linear * distance + light.Quadratic * distance * distance);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);
    vec3 specular = light.Color * spec;
	specular*=attenuation;
	diffuse*=attenuation;
	lighting+=diffuse+specular;

	FragColor =  vec4(lighting,1.0);
	
}