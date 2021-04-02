#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#include "./include/Filter.h"
#include"./include/VertexAttribute.h"

layout (location = 0) out vec2 uv00;
layout(location=1)out vec4 uv01;
layout(location=2)out vec4 uv23;

void main() {
	vec2 uv=TexUV;
		uv00 = TexUV;
		vec4 off=_MainTex_TexelSize.xyxy*vec4(1,1,-1,-1);
		//uv01.xy = uv - _MainTex_TexelSize.xy*vec2(1,1) * radius;//top right
		uv01.xy = uv - off.xy;//top right
		//uv01.zw = uv +_MainTex_TexelSize.xy* vec2(1,1) * radius;//bottom left
		uv01.zw = uv + off.xy;//bottom left
		//uv23.xy = uv - _MainTex_TexelSize.xy* vec2(1, -1) * radius;//top left
		uv23.xy = uv - off.xw;//top left
		//uv23.zw = uv +_MainTex_TexelSize.xy*  vec2(1, -1) * radius;//bottom right
		uv23.zw = uv + off.xw;//bottom right
		vec4 position=vec4(Position,1);
		gl_Position = position;

}