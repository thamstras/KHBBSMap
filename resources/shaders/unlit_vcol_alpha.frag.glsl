#version 330

uniform float alpha;

in vec4 vColor;

out vec4 FragColor;

void main()
{
	FragColor = vec4(vColor.rgb, alpha);
}