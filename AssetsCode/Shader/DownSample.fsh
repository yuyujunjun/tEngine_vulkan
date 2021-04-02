#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#include "./include/Filter.h"

layout (location = 0) in vec2 uv;
layout (location = 1) in vec4 uv01;
layout (location = 2) in vec4 uv23;
layout (location = 0) out vec4 outColor;

vec3 DownsampleAntiFlickerFilter()
{
	mediump vec3 s1 = texture(_MainTex, uv).rgb;//*4.0;
	mediump vec3 s2 = texture(_MainTex, uv01.xy).rgb;
	mediump vec3 s3 = texture(_MainTex, uv01.zw).rgb;
	mediump vec3 s4 = texture(_MainTex, uv23.xy).rgb;
	mediump vec3 s5 = texture(_MainTex, uv23.zw).rgb;
	//return (s1*4+s2+s3+s4+s5)/8.0;
	// Karis's luma weighted average (using brightness instead of luma)
	mediump float s1w = 1 / (Brightness(s1) + 1.0);
	mediump float s2w = 1 / (Brightness(s2) + 1.0);
	mediump float s3w = 1 / (Brightness(s3) + 1.0);
	mediump float s4w = 1 / (Brightness(s4) + 1.0);
	mediump float s5w = 1 / (Brightness(s5) + 1.0);
	mediump float one_div_wsum = 1 / (4.0*s1w + s2w + s3w + s4w + s5w);
	//mediump float one_div_wsum = 1 / ( s2w + s3w + s4w+s5w);
	return (s1 * s1w*4.0 + s2 * s2w + s3 * s3w + s4 * s4w + s5 * s5w) * one_div_wsum;
	//  return (s2 * s2w + s3 * s3w + s4 * s4w+s5*s5w) * one_div_wsum;
}

void main() {

    outColor.rgb = density* DownsampleAntiFlickerFilter();
	outColor.a=1;
	//outColor.rgb= texture(_MainTex, uv).rgb;
//	outColor.rgb=min(outColor.rgb,vec3(1000));
  //   mediump vec3 s1 = texture(_MainTex, uv).rgb*4.0;
  //   mediump vec3 s2 = texture(_MainTex, uv01.xy).rgb;
  //   mediump vec3 s3 = texture(_MainTex, uv01.zw).rgb;
  //   mediump vec3 s4 = texture(_MainTex, uv23.xy).rgb;
 	// mediump vec3 s5 = texture(_MainTex, uv23.zw).rgb;
  //   outColor.xyz=(s1+s2+s3+s4+s5)*0.125;
}
