#pragma once
#include"Sampler.h"
namespace tEngine {
	class tShader;
	class tDescriptorSetAllocator;
	using DescSetAllocHandle = std::shared_ptr<tDescriptorSetAllocator>;
	class tDescriptorSet;
	using DescriptorSetHandle = std::shared_ptr<tDescriptorSet>;
	class tBuffer;
	using BufferHandle = std::shared_ptr<tBuffer>;
	class tImage;
	using ImageHandle = std::shared_ptr<tImage>;// ::SharedPtr;
	class BindingResourceInfo;
	using ResSetBinding = std::vector<BindingResourceInfo>;
	class tShaderInterface {
	public:

		friend class CommandBuffer;
		tShaderInterface(const tShader* shader);
		//只是缓存，仅有拷贝操作
		void SetBuffer(std::string name, BufferHandle buffer, uint32_t offset = 0);
		//fake viewInfo
		void SetImage(std::string name, ImageHandle image, vk::ImageView vkView = {}, StockSampler sampler = StockSampler::LinearClamp);
		template <typename Attribute>
		void SetValue(std::string valueName, const Attribute& value, BufferHandle buffer = nullptr) {
			SetValue(valueName, &value, sizeof(Attribute), buffer);

		}


		template<typename Attribute>
		void SetPushConstant(std::string name, Attribute value) {
			SetPushConstant(name, &value, sizeof(Attribute));
		}

		const tShader* getShader() const {
			return base_shader;
		}


		size_t setCount()const {
			return bindResources.size();
		}
		const tShader* getShader() {
			return base_shader;
		}
		const DescSetAllocHandle& getDescSetAllocator(uint32_t set_number)const;

		const std::vector<uint8_t>& getPushConstantBlock()const;
		const std::vector<ResSetBinding>& getResSetBinding()const ;
		std::vector<ResSetBinding>& getResSetBinding();
		bool isSetEmpty(int i);
		const Device* getDevice();
	protected:

		std::vector<ResSetBinding> bindResources;
		//std::vector<PipelineBinding> bindResources;
		std::vector<uint8_t> pushConstantBlock;
	private:
		void SetPushConstant(std::string name, const void* attribute, size_t size);
		void SetValue(std::string valueName, const void* value, size_t size, BufferHandle buffer);
		const tShader* base_shader;


	};
	class CommandBuffer;
	using CommandBufferHandle = std::shared_ptr <CommandBuffer>;
	class tRenderPass;
	struct GraphicsState;
	void flushDescriptorSet(const CommandBufferHandle& cb, tShaderInterface& state);
	void flushGraphicsPipeline(const CommandBufferHandle& cb, GraphicsState& gState, tShaderInterface& state, tRenderPass* renderPass, uint32_t subpass);
	void flushComptuePipeline(const CommandBufferHandle& cb, tShaderInterface& state);
	void flushGraphicsShaderState(tShaderInterface* state, GraphicsState& gState, CommandBufferHandle& cb, tRenderPass* renderPass, uint32_t subpass);
	void flushComputeShaderState(tShaderInterface* state, CommandBufferHandle& cb);
	void fillWithDirtyImage(ResSetBinding& setBindings,const Device* device);
	void collectDescriptorSets(std::vector<DescriptorSetHandle>& bindedSets, std::vector<uint32_t>& offsets,
		const ResSetBinding& setBindings, const DescSetAllocHandle& setAllocator);
}