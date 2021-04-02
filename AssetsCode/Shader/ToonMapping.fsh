#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#include "./include/Filter.h"

//layout(set=2,binding=0)uniform sampler2D BrightnessImage;
layout (location = 0) in vec2 uv;
layout (location = 0) out vec4 outColor;


//#define Screen
void main() {
    
  
    vec4 brightness=texture(_MainTex,uv);
  	
#ifdef Screen
 	// color.xyz=ACESToneMapping(color.xyz,1);
 	// brightness.xyz=ACESToneMapping(intensity* brightness.xyz,1);
    // outColor=1-(1-brightness)*(1-color);
#else
   	vec3 blend=((intensity*brightness.xyz));
   	outColor.xyz=ACESToneMapping(blend,1);
	outColor.w=1;
#endif
 //	outColor=brightness;
}
