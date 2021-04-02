#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable



#include"./include/VertexAttribute.h"
layout (location = 0) out vec2 uv_;
void main() {
		vec4 position=vec4(Position,1);
		gl_Position = position;
        uv_ = TexUV;
}