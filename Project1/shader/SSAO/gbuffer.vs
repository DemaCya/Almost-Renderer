#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main(){

	vec4 viewPos = view * model * vec4(aPos, 1.0f);
    vs_out.FragPos = viewPos.xyz; 
    gl_Position = projection * viewPos;
    vs_out.TexCoords = aTexCoords;
    
    mat3 normalMatrix = transpose(inverse(mat3(view * model)));
    vs_out.Normal = normalMatrix * aNormal;

}