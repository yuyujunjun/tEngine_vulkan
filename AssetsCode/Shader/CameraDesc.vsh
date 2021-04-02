#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#include"./include/VertexAttribute.h"

layout(location=0)out vec4 color;

void main() {
	vec4 position=vec4(Position,1);
	gl_Position = _MATRIX_VP * position;
	gl_Position.y=-gl_Position.y;
}