#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
//#include "./include/Global.h"
#include"./include/VertexAttribute.h"



layout (location = 0) out vec2 uv_;
layout(location=1) out vec4 color;
layout(location=2) out vec3 worldNormal;
layout(location=3) out vec3 worldPosition;
void main() {


		vec4 position=vec4(Position,1);
		worldPosition=(_MATRIX_M*position).xyz;
		gl_Position = _MATRIX_VP * vec4(worldPosition,1);
		worldNormal = (_MATRIX_V*_MATRIX_M*vec4(Normal,0)).xyz;
		
		//normal=Normal;
	//	gl_Position.y=-gl_Position.y;
        uv_ = TexUV;
		color=Color;
}