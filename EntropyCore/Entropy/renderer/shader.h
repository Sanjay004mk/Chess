#pragma once
#include "Entropy/EntropyUtils.h"
#include "render_info.h"
#include "push_constant.h"

#define ET_SHADER_DEFAULT_ARRAY_SIZE 128

namespace et
{
	class Texture;

	struct ShaderCreateInfo
	{
		std::vector<VertexInputAttribute> vertexAttributes;
		std::vector<VertexInputBinding> vertexBindings;
		std::vector<UniformDescription> uniformDescriptions;
		std::vector<ShaderData> pushConstants;
	};

	class Shader
	{
	public:
		Shader() = default;
		Shader(const std::string& name) : name(name) {}
		virtual ~Shader() {}

		virtual void SetUniform(const std::string& uniformName, const void* data) = 0;
		virtual void SetTextures(const std::string& uniformName, const std::vector<Ref<Texture>>& textures) = 0;
		virtual void UploadPushConstant() = 0;

		virtual void Bind(PipelineBindPoint bindPoint = PipelineBindPoint::Graphics, uint32_t dynamicOffsetCount = 0, uint32_t* pDynamicOffsets = nullptr) = 0;
		virtual void UnBind() {}

		std::string name{};
		std::vector<VertexInputAttribute> vertexAttributes;
		std::vector<VertexInputBinding> vertexBindings;
		PushConstant pushConstant;
		
		static const ShaderCreateInfo BasicShaderCreateInfo;
	};


	Ref<Shader> CreateShader(const std::string& fileName);
	Ref<Shader> CreateShader(const std::string& fileName, const ShaderCreateInfo& createInfo);
	Ref<Shader> CreateShader(std::stringstream& source);
	Ref<Shader> CreateShader(std::stringstream& source, const ShaderCreateInfo& createInfo);
}