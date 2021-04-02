#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
//#include "./include/Global.h"
#include"./include/VertexAttribute.h"
layout(location=0) out vec3 worldPosition;
void main() {
		gl_Position = _MATRIX_VP * _MATRIX_M*vec4(Position,1);
		//gl_Position.w=1000;
		worldPosition=(_MATRIX_V*_MATRIX_M*vec4(Position,1)).xyz;

}