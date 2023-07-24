# My Vulkan Library

My own Vulkan library aims to simplify the process of writing Vulkan code. With my library, all of the Vulkan objects are created semi-automatically, and some are transparent to the programmers.

Transparent Vulkan objects are created just before they are used in the context. I use time-relevant heaps to store objects, with the most recently used objects at the top of the heap. If a usable object is already in the heap, it is reused instead of creating a new one. This strategy also helps avoid creating equivalent objects.

Here are some examples of specific situations where this strategy is used:

- To use a shader for different meshes with different materials, the descriptorSets must be different. However, if the types of shader parameters are the same, I distinguish the descriptorSets by the specific resources. When the resources are the same, the same descriptorSet is used.
- To use a renderPass with different imageviews, different frameBuffers are needed. However, if the rendering logic is the same, I bind the frameBufferHeap to the renderPass. If there is a group with the same resources, I find and use the required frameBuffer from the frameBufferHeap.

I have also simplified the process of filling descriptorSets. Firstly, my library gathers all shader parameters via ```CompileRef.bat```, and the ```Shader``` class automatically fills them into the descriptorSetLayout (just before binding the descriptorSets). The program can automatically create the uniform buffer from the information provided by the uniform block property of the shader. I use a dynamic DescriptorSet for the uniform buffer, which has a size of 64 at first. I use it as the ring buffer. If there is a change in the uniform buffer, I offset the memory of the buffer. In cases where many instances of an object need to be rendered, creating the uniform buffer manually can be more effective.

Finally, I have included a naive rigid-based physical engine (still in testing) and an ECS manager to manage all the GameObjects for rendering and simulating.
