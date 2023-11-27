#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D planeTexture;

void main(){
	vec4 texColor = texture(planeTexture,TexCoords);
	if(texColor.a<0.1)
		discard;
	texColor = vec4(0.5,0.5,0.5,texColor.a);
	FragColor = texColor;
}