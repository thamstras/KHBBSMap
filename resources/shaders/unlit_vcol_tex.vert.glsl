#version 330

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 tex;

out vec4 vColor;
out vec2 vTex;
out float vDepth;

void main()
{
	gl_Position = projection * view * model * position;
	vColor = color;
	vTex = tex;
	vDepth = -(view * model * position).z;
}