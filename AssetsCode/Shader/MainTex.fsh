#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable


layout(set = 1, binding = 3)uniform sampler2D _MainTex;

layout (location = 0) in vec2 uv;
layout (location = 0) out vec4 outColor;
//layout(location=1)out vec4 debugColor;
float unpack_depth(const in vec4 rgba_depth)
{
    const vec4 bit_shift = vec4(1.0/(256.0*256.0*256.0), 1.0/(256.0*256.0), 1.0/256.0, 1.0);
    float depth = dot(rgba_depth, bit_shift);
    return depth;
}
void main() {
   
	float depth=texture(_MainTex,uv).r;
	//depth=(depth-0.98)/0.02;
	outColor = vec4(depth);
	
	//outColor=vec4(1,1,1,1);
	//debugColor=vec4(1,0,1,1);
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
