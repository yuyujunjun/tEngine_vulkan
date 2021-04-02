#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#include "./include/Filter.h"

layout(location=0)in vec4 uv01;
layout(location=1)in vec4 uv23;
layout(location=2)in vec4 uv45;
layout(location=3)in vec4 uv67;
layout (location = 0) out vec4 outColor;

void main() {

    

    vec4 sum=vec4(0,0,0,0);
    
        
    sum += texture(_MainTex, uv01.xy);
      
    sum += texture(_MainTex, uv01.zw) * 2.0;

       
    sum += texture(_MainTex, uv23.xy);

    sum += texture(_MainTex, uv23.zw) * 2.0;

        
    sum += texture(_MainTex, uv45.xy);

        
    sum += texture(_MainTex, uv45.zw) * 2.0;

         
    sum += texture(_MainTex, uv67.xy);

          
    sum += texture(_MainTex, uv67.zw) * 2.0;

        
    outColor = sum*0.0833;
    outColor.w=1;
   // outColor=texture(_MainTex, uv23.zw) ;
    //outColor=texture(_MainTex, uv01.xy);
}
