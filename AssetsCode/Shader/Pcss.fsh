#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout (location = 0) in vec2 uv;
layout(location=1)in vec4 color;
layout(location=2) in vec3 worldNormal;
layout(location=3)in vec3 worldPosition;
layout (location = 0) out vec4 outColor;
layout(set = 1, binding = 0)uniform sampler2D _MainTex;
layout(set = 1,binding = 2)uniform sampler2D _ShadowMap;
layout(set = 1, binding = 3) uniform MaterialInfo {
   
    vec3 uKd;float pad0;
    vec3 uKs;float pad1;
    vec4 lightPosArea;
    float lightIntensity;
};
layout(set = 0,binding = 2)uniform globalVar{
    vec3 cameraPos;int halfblockSize;
    mat4 world_to_shadow;
    vec2 depthMapSize;int maxKernelSize;

};
float unpack_depth(const in vec4 rgba_depth)
{
    const vec4 bit_shift = vec4(1.0/(256.0*256.0*256.0), 1.0/(256.0*256.0), 1.0/256.0, 1.0);
    float depth = dot(rgba_depth, bit_shift);
    return depth;
}
float CmpTexture(vec2 uv,float v){
    return texture(_ShadowMap,uv).r<v?0.f:1.f;
}

float calculateShadowFactor(vec2 uv,float depth,int kernelSize){
    vec2 offset=vec2(1.0/depthMapSize.x,1.0/depthMapSize.y);
    
    float d0=0;
    int count=0;
    for(int i=-kernelSize;i<=kernelSize;++i){
        for(int j=-kernelSize;j<=kernelSize;++j){
            d0+=CmpTexture(uv+vec2(i*offset.x,j*offset.y),depth);
            count++;
        }
    }
    float area=2*kernelSize+1;
    area=area*area;
    d0/=count;
    return d0;
    

}
vec2 avgOccdepth(vec2 uv,float depth){
 vec2 offset=vec2(1.0/depthMapSize.x,1.0/depthMapSize.y);
    float occAvg=0;
    int count=0;
    for(int i=-halfblockSize;i<=halfblockSize;++i){
        for(int j=-halfblockSize;j<=halfblockSize;++j){
            float d=texture(_ShadowMap,uv+vec2(i*offset.x,j*offset.y)).r;
            if(d<depth){
                occAvg+=d;
                count++;
            }

        }
    }
    
    occAvg/=float(count);
    return vec2(occAvg,float(count));
}
float pcss(vec2 uv,float depth){
  //  return float(depth<unpack_depth(texture(_ShadowMap,uv)));
    vec2 occDepth=avgOccdepth(uv,depth);
    if(occDepth.y==0)return 1;
    float occAvg=occDepth.x;
    float penmubraSize= (lightPosArea.w*(depth-occAvg)/occAvg);
    
    penmubraSize=max(min(penmubraSize,1.f),0);
    int kernelSize=int(maxKernelSize*penmubraSize+0.5);
   // kernelSize=max(kernelSize,1);
   // return kernelSize/float(maxKernelSize);
     
    return calculateShadowFactor(uv,depth,kernelSize);
}

void main() {
//   outColor = vec4(texture(_MainTex,uv));
    vec3 color=pow(texture(_MainTex,uv).xyz,vec3(2.2));
    vec3 ambient=0.05*color;
    
    vec3 lightDir=(lightPosArea.xyz-worldPosition);
    float distance_to_light=length(lightDir);
    lightDir=lightDir/distance_to_light;
    vec3 normal=normalize(worldNormal);
    float diff=max(dot(lightDir,normal),0.0);
    float light_atten_coff=lightIntensity;
    vec3 diffuse=diff*light_atten_coff*color;

    vec3 viewDir=normalize(cameraPos-worldPosition);
    float spec=0.0;
    vec3 reflectDir=2*dot(normal,lightDir)*normal-lightDir;
    spec=pow(max(dot(viewDir,reflectDir),0.0),35.0);
    vec3 specular=uKs*light_atten_coff*spec;
	
    //shadow
    vec4 position_in_light=world_to_shadow*vec4(worldPosition,1);
    position_in_light.y=- position_in_light.y;
    vec2 uv_shadow=position_in_light.xy*0.5+0.5;
    float light_z=position_in_light.z;
    float shadowFactor = pcss(uv_shadow,light_z);
    vec3 lighten=(diffuse+specular)*shadowFactor;
    outColor=  vec4(pow(ambient+lighten,vec3(1.0/2.2)),1.0);
  //  outColor=vec4(position_in_light.z);
  //  outColor= vec4(shadowFactor);
//    outColor=vec4(nearest_depth);
// outColor=vec4(uv_shadow,0,1);
}
