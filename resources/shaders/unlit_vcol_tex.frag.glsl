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
	
	// Some textures seem to use 0x80 to represent 1.0 alpha.
	// This gives an alpha value of ~0.5 (128/255)
	// So we need to multiply our alpha by the inverse of that (255/128)
	// to recover the original value.
	// TODO: It would be better to figure out what textures do this and fix the alpha
	//       during decode rather than multiply all alpha channels.
	// NOTE: This float value is exact
	texColor.a = texColor.a * 1.9921875f;

	FragColor = vColor * texColor;
	//FragColor = vColor;
	//FragColor = texture2D(tex_diffuse, vTex);
}