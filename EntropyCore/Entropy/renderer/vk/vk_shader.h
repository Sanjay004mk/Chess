#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vulkan/vulkan.h>

#include "../shader.h"

namespace et
{
	class BufferArray;

	/*class VulkanShader : public Shader
	{
	public:
		VulkanShader(const std::string& fileName, const std::string& name = std::string(), VertexBindFn bind_fn = Shader::GetVertexBindDesc, VertexAttrFn attr_fn = Shader::GetVertexAttrDesc);
		VulkanShader(const char* file_name, VkRenderPass render_pass, const ShaderCreateInfo& create_info = ShaderCreateInfo(), const std::string& name = std::string(), VertexBindFn bind_fn = Shader::GetVertexBindDesc, VertexAttrFn attr_fn = Shader::GetVertexAttrDesc);
		~VulkanShader();

		void CreateShaderModule(VkRenderPass render_pass, const ShaderCreateInfo& create_info);
		void ResetPipeline(VkRenderPass render_pass, const ShaderCreateInfo& create_info);
		void SetUniform(const std::string& uniform_name, const void* data);

		operator VkPipeline() const { return pipeline; }
		operator VkPipelineLayout() const { return pipeline_layout; }
		VkDescriptorSet GetDescSet(int32_t i) const { return set[i]; }

	private:
		void Init(const char* file_name);
		void ExtractAttributesAndUniforms(ShaderType stage, const std::string& shader_source);
		void SetupDescriptors();
		void DestroyPipeline();

		static VkVertexInputBindingDescription GetVertexBindDesc();
		static std::vector<VkVertexInputAttributeDescription> GetVertexAttrDesc();

		std::string name;
		std::unordered_map<ShaderType, std::vector<uint32_t>> shaderSPIRV;
		std::unordered_map<ShaderType, std::unordered_map<uint32_t, std::pair<std::string, std::vector<ShaderData>>>> attributes;
		std::unordered_map<ShaderType, std::unordered_map<uint32_t, std::pair<std::string, std::vector<ShaderData>>>> uniforms;

		std::vector<VkDescriptorSet> set;
		VkDescriptorPool pool = VK_NULL_HANDLE;
		VkDescriptorSetLayout layout = VK_NULL_HANDLE;
		VkPipeline pipeline = VK_NULL_HANDLE;
		VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
		std::unordered_map<std::string, uint32_t> buffer_indices;
		std::vector<BufferArray*> buffers;
	};*/

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