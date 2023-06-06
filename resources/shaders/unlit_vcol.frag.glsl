#version 330

in vec4 vColor;

out vec4 FragColor;

void main()
{
	FragColor = vColor;
	//FragColor = vec4(vColor.a, vColor.a, vColor.a, 1.0);
}