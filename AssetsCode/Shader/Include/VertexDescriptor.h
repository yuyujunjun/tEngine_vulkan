#ifndef VERTEXDESCRIPTOR
#define VERTEXDESCRIPTOR
layout(set = 0, binding = 0) uniform CameraMatrix {
	mat4 _MATRIX_V;
	mat4 _MATRIX_P;
	mat4 _MATRIX_VP;
	mat4 _INV_MATRIX_VP;
};

layout(set=2,binding=0) uniform ModelMatrix{
	mat4 _MATRIX_M;
};
#endif