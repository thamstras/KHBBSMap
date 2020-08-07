#version 330

uniform sampler2D tex_diffuse; 

in vec4 vColor;
in vec2 vTex;

out vec4 FragColor;

void main()
{

	vec4 texColor = texture2D(tex_diffuse, vTex);
	if (texColor.a <= 0.01f)
		discard;
	
	FragColor = vColor * texColor;
	//FragColor = vColor;
	//FragColor = texture2D(tex_diffuse, vTex);
}