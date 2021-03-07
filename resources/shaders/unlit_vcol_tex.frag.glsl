#version 330

uniform sampler2D tex_diffuse;
uniform vec4 fog_color;
uniform float fog_near;
uniform float fog_far;
uniform vec2 uv_offset;

in vec4 vColor;
in vec2 vTex;
in float vDepth;

out vec4 FragColor;

void main()
{

	vec4 texColor = texture2D(tex_diffuse, vTex + uv_offset);
	
	vec4 finalColor = texColor * vColor;
	// NOTE: This float value is exact
	//finalColor.a = finalColor.a * 1.9921875f;
	
	if (finalColor.a < 0.01f)
		discard;
	
	float trueAlpha = finalColor.a;
	finalColor.a = min(finalColor.a, 1.0);
	// TODO: use trueAlpha for something
	
	//finalColor.r = finalColor.r * finalColor.a;
	//finalColor.g = finalColor.g * finalColor.a;
	//finalColor.b = finalColor.b * finalColor.a;

	float fogAmount = smoothstep(fog_near, fog_far, vDepth);

	FragColor = mix(finalColor, fog_color, fogAmount);
	FragColor.a = finalColor.a;
}