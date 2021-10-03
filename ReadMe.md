My own vulkan library.

### Reduce the pain of wrting vulkan codes

All of the vulkan objects are created semi-automatically, and some of them are totally transparent to the pragrammers.  

Note that the transparent vulkan objects will be created just before they are used in the context.   I create some time-relevant heaps which the last used objects will be at the top of the heap. If there is a useable object in that heap, I just use it instead of creating a new one. This strategy also avoid creating the equivalent objects.

Now I will point out some specific situations which use this strategy for examples.

1. We could use a shader for different mesh, with different material, which means the descriptorSets must be different. However, the type of shader's parameters are the same. So I distinguish the discriptorSets by the specific resources and when the resources are the same, I use the same discriptorSet.
2. We could use a renderPass with different imageviews, which means we need different frameBuffers. However, the logic of rendering is the same. So I bind the frameBufferHeap to the renderPass, if there is a group of the same resources, I find the required frameBuffer from the frameBufferheap.



### Reduce the pain of filling descriptorSet 

Firstly , the library gather all shader's parameters by ```CompileRef.bat``` and ```Shader``` class will fill them into descriptorSetLayout automatically ( just before binding the descriptorSets).

From the information provided by uniform block property of the shader, our program can automatically create the uniform buffer. I want to use the dynamic DescriptorSets for uniform buffer so I create a uniform buffer which has a bigger size (64 $\times$ size) at first. I will use it as the ring buffer.  If there will be a changing happens in the uniform buffer, I just offset the memory of the buffer.  If you need to render many instances of a object, a better way is to create the uniform buffer manually.

### Others

I write a naive rigid-based physical engine (not stable yet).

I write a naive ECS manager to manage all the GameObjects for rendering and simulating. 





 