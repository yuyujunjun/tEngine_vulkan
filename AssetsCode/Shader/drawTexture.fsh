#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 1, binding = 1)uniform _MainTexTexelSize1
{
	vec4 _MainTex_TexelSize;
};

layout(set = 1, binding = 3)uniform sampler2D _MainTex;

layout (location = 0) in vec2 uv;
layout(location=1)in vec4 color;
layout(location=2) in vec3 normal;
layout(location=3)in vec3 worldPosition;
layout (location = 0) out vec4 outColor;


void main() {
   
	vec3 abeldo=vec3(1,1,1);
	float LdotV=abs(dot(normalize(normal),vec3(0,0,1)));
	outColor=  vec4(LdotV*abeldo*0.7+abeldo*0.3,1);
	outColor = vec4(texture(_MainTex,uv));
	//outColor=vec4(normalize(normal),1);
	//brightnessColor=vec4(1);
	//brightnessColor.xyz= Prefilter2(outColor.xyz);
	
	// brightnessColor=vec4(0);
	// if(Brightness(outColor.xyz)>0.9){
	// 	brightnessColor=outColor;
	// }
	//outColor=EncodeHDR(outColor.xyz);
	//brightnessColor=EncodeHDR(brightnessColor.xyz);
}
