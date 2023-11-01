#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

uniform vec3 camPos;
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;
const float PI = 3.14159265359;

uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];
uniform samplerCube irradianceMap;


float D_GGX_TR(vec3 N,vec3 H,float a){
	float NdotH = max(dot(N,H),0.0);
	float NdotH2 = NdotH*NdotH;
	float a2 = a*a;
	float bottom = (NdotH2 * (a2 - 1.0) + 1.0)*(NdotH2 * (a2 - 1.0) + 1.0);
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    //denom = PI * denom * denom;
	float final = a2/(PI*denom*denom);
	//return a2 / (PI * denom * denom);
	return final;
}

float G_SchlickGGX(float NdotV,float K){

	float r = (K + 1.0);
    float k = (r*r) / 8.0;

	float bottom = NdotV * (1.0 - k) + k;
	return NdotV/bottom;
}

float Smith_G(vec3 N,vec3 V,vec3 L,float K){
	float NdotV = max(dot(N,V),0.0);
	float NdotL = max(dot(N,L),0.0);

	float ggx1 = G_SchlickGGX(NdotV,K);
	float ggx2 = G_SchlickGGX(NdotL,K);
	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
} 

void main(){
	vec3 N = normalize(Normal);
	vec3 V = normalize(camPos - WorldPos);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0,albedo,metallic);

	vec3 Lo = vec3(0.0);
	for(int i=0;i<4;i++){

		vec3 N = Normal;
		vec3 L = normalize(lightPositions[i] - WorldPos);
		vec3 H = normalize(L + V);
		float distance = length(lightPositions[i] - WorldPos);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = lightColors[i] * attenuation;

		

		vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
		float NDF = D_GGX_TR(N,H,roughness);
		float G = Smith_G(N,V,L,roughness);

		vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;
        
        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;	  

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}
	vec3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness); 
	vec3 kD = 1.0 - kS;
	vec3 irradiance = texture(irradianceMap, N).rgb;
	vec3 diffuse    = irradiance * albedo;
	vec3 ambient    = (kD * diffuse) * ao; 

	vec3 color = ambient + Lo;

	color = color / (color + vec3(1.0));

	color = pow(color, vec3(1.0/2.2));

	FragColor = vec4(color,1.0);
}