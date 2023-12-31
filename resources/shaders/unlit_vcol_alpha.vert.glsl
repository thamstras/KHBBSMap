#version 330

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 color;

out vec4 vColor;

void main()
{
	gl_Position = projection * view * model * position;
	vColor = color;
}