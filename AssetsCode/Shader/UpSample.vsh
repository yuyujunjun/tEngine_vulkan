#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#include "./include/Filter.h"


#include"./include/VertexAttribute.h"

layout(location=0)out vec4 uv01;
layout(location=1)out vec4 uv23;
layout(location=2)out vec4 uv45;
layout(location=3)out vec4 uv67;


void main() {
	vec2 uv=TexUV;
		vec2 off=_MainTex_TexelSize.xy;
		uv01.xy = uv + off*vec2(-2,0) 			;
		uv01.zw = uv + off*vec2(-1,1) 	        ;
		uv23.xy = uv + off*vec2(0, 2) 			;
		uv23.zw = uv + off 	                   ;
		uv45.xy = uv + off*vec2(2, 0) 			;
		uv45.zw = uv + off*vec2(1, -1) 	        ;
		uv67.xy = uv + off*vec2(0, -2) 			;
		uv67.zw = uv + off*vec2(-1, -1)           ;
		vec4 position=vec4(Position,1);
		gl_Position = position;

}