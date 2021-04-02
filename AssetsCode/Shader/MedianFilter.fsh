#version 430
//#extension GL_ARB_separate_shader_objects : enable
//#extension GL_ARB_shading_language_420pack : enable
#include "./include/Filter.h"

layout (location = 0) in vec2 uv;
layout (location = 0) out vec4 outColor;
mediump vec3 frag_medium(vec2 uv)
{
	//  vec2 uv = i.uv + _MainTex_TexelSize.xy * _PrefilterOffs;
	vec3 d = _MainTex_TexelSize.xyx * vec3(0.5, 0.5, 0);
	mediump vec3 s0 = texture(_MainTex, uv).rgb;
	mediump vec3 s1 = texture(_MainTex, uv - d.xz).rgb;
	mediump vec3 s2 = texture(_MainTex, uv + d.xz).rgb;
	mediump vec3 s3 = texture(_MainTex, uv - d.zy).rgb;
	mediump vec3 s4 = texture(_MainTex, uv + d.zy).rgb;
	
	mediump vec3 m = Median(Median(s0.rgb, s1, s2), s3, s4);
	return m;
}

void main() {
    outColor.w=1;
    outColor.xyz=frag_medium(uv);
    mediump float brightness=Brightness(outColor.xyz);
    mediump float soft=brightness-_Filter.y;
    soft=clamp(soft,0,_Filter.z);
    soft=soft*soft*_Filter.w;
    mediump float contribution=max(soft,brightness-_Filter.x);
    contribution/=max(brightness,0.001);
    outColor.xyz*=contribution;
  //  outColor.xyz=max(outColor.xyz-vec3(_Filter.x),vec3(0));
}
