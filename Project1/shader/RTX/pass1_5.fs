#version 330 core

layout (location = 0) out vec3 Color;

in vec2 texcoord;
in vec3 pix;
uniform sampler2D frame;
void main(){
	Color = texture2D(frame, pix.xy*0.5+0.5).rgb;
}