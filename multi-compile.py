# -*- coding: utf-8 -*-
"""
Created on Wed Jul  8 20:23:47 2020

@author: View
"""

import os
import copy
import json
import shutil
shader_dir="./AssetsCode/Shader/"
spv_dir="./Assets/Shader/"
json_dir="./Assets/Json/"
different_define_json_dir="./Assets/Shader/json/"
#编译shader文件
def CompileSPV(shader,spv,jsondir):
    path_dir = os.getcwd()    #exe文件位置
    cmd=""
    for fn in os.listdir(os.path.join(path_dir,shader)):
        if fn[-4:]==".fsh":
            cmd+="glslc.exe -O0 -x glsl -fshader-stage=frag %s -o %s"%(shader+fn,spv+fn+".spv")+"&&"
            #cmd+="spirv-opt.exe -O %s -o %s"%(spv+fn+".spv",spv+fn+".spv")+"&&"
        
        if fn[-4:]==".vsh":
            cmd+="glslc.exe -O0 -x glsl -fshader-stage=vert %s -o %s"%(shader+fn,spv+fn+".spv")+"&&"
            #cmd+="spirv-opt.exe -O %s -o %s"%(spv+fn+".spv",spv+fn+".spv")+"&&"
        
        if fn[-4:]==".csh":
            cmd+="glslc.exe -O0 -x glsl -fshader-stage=comp %s -o %s"%(shader+fn,spv+fn+".spv")+"&&"
            #cmd+="spirv-opt.exe -O %s -o %s"%(spv+fn+".spv",spv+fn+".spv")+"&&"
    os.system(cmd[:-2])
def optimalSPV(shader,spv,jsondir):
    path_dir = os.getcwd()    #exe文件位置
    print("Start Optimial")
    cmd=""
    for fn in os.listdir(os.path.join(path_dir,shader)):
       # print(fn)
        if fn[-4:]==".fsh":
            #cmd+="glslc.exe -x glsl -fshader-stage=frag %s -o %s"%(shader+fn,spv+fn+".spv")+"&&"
            cmd+="spirv-opt.exe -O %s -o %s"%(spv+fn+".spv",spv+fn+".spv")+"&&"
        
        if fn[-4:]==".vsh":
            #cmd+="glslc.exe -x glsl -fshader-stage=vert %s -o %s"%(shader+fn,spv+fn+".spv")+"&&"
            cmd+="spirv-opt.exe -O %s -o %s"%(spv+fn+".spv",spv+fn+".spv")+"&&"
        
        if fn[-4:]==".csh":
            #cmd+="glslc.exe -x glsl -fshader-stage=comp %s -o %s"%(shader+fn,spv+fn+".spv")+"&&"
            cmd+="spirv-opt.exe -O %s -o %s"%(spv+fn+".spv",spv+fn+".spv")+"&&"
    os.system(cmd[:-2])
    print("End Optimial")
def writeDifferentDefineShader(shaderdir):
    for file in os.listdir(shaderdir):
        if file[-4:] in {".vsh",".csh",".fsh"}:      #如果是shader文件
            json_data={}
            defineProductList=[]  
            json_data["name"]=[]
            readShader(file,json_data,defineProductList)
            
            
#根据json文件写相应include
def writeSingleShader(defineList,filename,data,json_data):
    json_data["size"]=len(defineList)
    for idx in range(len(defineList)):
        file=filename[:-4]+"_"+str(idx)+filename[-4:]
        json_data[idx]={}
        spv=spv_dir[9:]+file+".spv"
        refl=json_dir[9:]+file+".refl"
        json_data[idx]["spv"]=spv
        json_data[idx]["refl"]=refl
        
        addText=""
        for content in defineList[idx]:
            addText+="#define "+content+"\n"
            if content not in json_data:
                json_data["name"].append(content)
                json_data[content]=[]
            json_data[content].append(idx)
        text=data[:data.find("\n")+1]+addText+data[data.find("\n")+1:]

        with open(os.path.join(shader_dir,file),"w")as f:
            f.write(text)
        f.close()

      #  print(file+".spv")
      #  print(defineList[idx])
        cmd=""
        if filename[-4:]==".fsh":
            cmd+="glslc.exe -O0 -x glsl -fshader-stage=frag %s -o %s"%(shader_dir+file,spv_dir+file+".spv")#+"&&"
        if filename[-4:]==".vsh":
            cmd+="glslc.exe -O0 -x glsl -fshader-stage=vert %s -o %s"%(shader_dir+file,spv_dir+file+".spv")#+"&&"
        if filename[-4:]==".csh":
            cmd+="glslc.exe -O0 -x glsl -fshader-stage=comp %s -o %s"%(shader_dir+file,spv_dir+file+".spv")#+"&&"
        #cmd+="spirv-opt.exe -O %s -o %s"%(spv_dir+file+".spv",spv_dir+file+".spv")
        os.system(cmd)
        os.remove(os.path.abspath(os.path.join(shader_dir,file)))
    with open(os.path.join(different_define_json_dir+filename+".json"),"w") as f:
        json.dump(json_data,f)

        
#组合include
def dfs(defineProductList,defineList,idx,ans):
    if(idx==len(defineList)):
        defineProductList.append(copy.deepcopy(ans))
        return
    for i in range(len(defineList[idx])):
        ans.append(defineList[idx][i])
        dfs(defineProductList,defineList,idx+1,ans)
        ans.pop()
        
    
#读取shader，根据“#pragma”生成需要组合的列表
def readShader(file,json_data,defineProductList):
    defineList=[]
    with open(os.path.join(shader_dir,file),"r")as f:
        shader=f.readlines()
    data=""
    for line in shader:
        if "#pragma" and "multiple" in line:
            defineLineList=[]
            temp=line.split("(")[1][:-2]
            for i in temp.split(" "):
                defineLineList.append(i)
            defineList.append(defineLineList)
        else:
            data+=line
    f.close()   
    if len(defineList)!=0:
        dfs(defineProductList,defineList,0,[])
        writeSingleShader(defineProductList,file,data,json_data)

def MKDIR(path):
    while(True):
        try:
            os.mkdir(path)
            break
        except PermissionError:
            print("try again")
            continue            
if __name__=="__main__":
    # if not os.path.exists(shader_dir):
    #     os.mkdir(shader_dir)
    if os.path.exists(spv_dir):
        shutil.rmtree(os.path.abspath(spv_dir))  
        MKDIR(spv_dir)
    else:
        os.mkdir(spv_dir)
    if os.path.exists(json_dir):
        shutil.rmtree(os.path.abspath(json_dir))   
        MKDIR(json_dir)
    else:
        os.mkdir(json_dir)
    if not os.path.exists(different_define_json_dir):
        os.mkdir(different_define_json_dir)
    else:
        shutil.rmtree(os.path.abspath(different_define_json_dir))   
        MKDIR(different_define_json_dir)
      
     
    
    
    CompileSPV(shader_dir,spv_dir,json_dir)
    writeDifferentDefineShader(shader_dir)
    
    os.system("refl.exe %s %s"%(spv_dir,json_dir))
    optimalSPV(shader_dir,spv_dir,json_dir)


