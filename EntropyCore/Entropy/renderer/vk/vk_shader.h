#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vulkan/vulkan.h>

#include "../shader.h"

namespace et
{
	class BufferArray;

	class VulkanShader : public Shader
	{
	public:
		VulkanShader(const std::string& name, const std::unordered_map<ShaderStage, std::string>& shaderSources);
		VulkanShader(const std::string& name, const std::unordered_map<ShaderStage, std::string>& shaderSources, const ShaderCreateInfo& createInfo);
		~VulkanShader();

		virtual void SetUniform(const std::string& uniformName, const void* data) override;
		virtual void SetTextures(const std::string& uniformName, const std::vector<Ref<Texture>>& textures) override;
		virtual void UploadPushConstant() override;

		virtual void Bind(PipelineBindPoint bindPoint, uint32_t dynamicOffsetCount, uint32_t* pDynamicOffsets) override;
		virtual void UnBind() override;

		std::vector<VkPipelineShaderStageCreateInfo> GetShaderStages() const;
		VkPipelineLayout GetPipelineLayout() const { return mPipelineLayout; }
		VkDescriptorSet GetSet(uint32_t index) const { return mSets[index]; }

	private:
		void CompileShaders(const std::unordered_map<ShaderStage, std::string>& shaderSources);
		ShaderCreateInfo ExtractInfo(const std::unordered_map<ShaderStage, std::string>& shaderSources);
		void SetupDescriptors(const ShaderCreateInfo& createInfo);

		std::vector<VkDescriptorSet> mSets;
		VkDescriptorPool mPool = VK_NULL_HANDLE;
		VkDescriptorSetLayout mLayout = VK_NULL_HANDLE;
		VkPipelineLayout mPipelineLayout;
		std::unordered_map<ShaderStage, VkShaderModule> mShaderModules;
		std::unordered_map<std::string, uint32_t> mUniformBindings;
		std::unordered_map<std::string, std::tuple<uint32_t, size_t>> mTextureBindings;
		std::vector<Ref<BufferArray>> mUniformBuffers;
	};
}