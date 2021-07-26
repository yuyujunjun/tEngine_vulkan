#pragma once
namespace tEngine {
	struct GraphicsState {
		bool depthTestEnable = true;
		bool depthWriteEnable = true;
		//Clamp depth to viewPort.minDepth->viewPort.maxDepth,if set it false, discard depth not in 0->gl_Position.w
		bool depthClampEnable = false;
		//useful for shadowMap, but I prefer calculate mannully
		struct DepthBias {
			bool depthBiasEnable = false;
			float depthBiasSlopeFactor;
			//related to numerical accuracy
			float depthBiasConstantFactor;
			//related to geometry gradient
			float depthBiasClamp;
		}depthBias;
		vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
	};
}