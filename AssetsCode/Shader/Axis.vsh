#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#include"./include/VertexAttribute.h"

layout(location=0)out vec4 color;

void main() {
		vec4 position=_MATRIX_M*vec4(Position,1);
        mat4 New_VP=_MATRIX_VP;
     //   New_VP[0].xyz=_MATRIX_VP[0].xyz/length(_MATRIX_VP[0].xyz);
     //   New_VP[1].xyz=_MATRIX_VP[1].xyz/length(_MATRIX_VP[1].xyz);
     //   New_VP[2].xyz=_MATRIX_VP[2].xyz/length(_MATRIX_VP[2].xyz);

		
		gl_Position = New_VP * position;
	//	gl_Position.y=-gl_Position.y;
       
		color = Color;

}