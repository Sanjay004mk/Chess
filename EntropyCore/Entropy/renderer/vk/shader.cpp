#include <etpch.h>
#include <filesystem>
#include <shaderc/shaderc.hpp>

#include "Entropy/scene/components.h"
#include "vkapi.h"

namespace et
{
	const std::vector<VertexInputAttribute> BasicVertexAttributes =
	{ // vertexAttributes
		{
			0,								// location
			0,								// binding
			DataType::Vec2,					// format
			offsetof(Vertex, position)		// offset
		},

		{
			1,								// location
			0,								// binding
			DataType::Vec2,					// format
			offsetof(Vertex, uv)			// offset
		},

		{
			2,								// location
			0,								// binding
			DataType::Vec3,					// format
			offsetof(Vertex, color)			// offset
		},

		{
			3,                              // location
			0,								// binding
			DataType::Int32,				// format
			offsetof(Vertex, texture_index)	// offset
		}
	};

	const std::vector<VertexInputBinding> BasicVertexBinding =
	{ // vertexBinding
		{
			0,								// binding
			sizeof(Vertex),					// stride
			VertexInputRate::PerVertex
		}
	};

	const ShaderCreateInfo Shader::BasicShaderCreateInfo =
	{
		BasicVertexAttributes,
		BasicVertexBinding,
		{
			{
				ShaderStage::Vertex,
				0,								// binding
				ShaderDataType::Mat4f,			// bindingInfo
				"vp_mat"						// name
			}
		},
		{									// push constants
			ShaderDataType::Mat4f,
			ShaderDataType::Int,
			ShaderDataType::Int,
		}
	};

	static std::string GetSource(std::string_view fileName)
	{
		std::ifstream file(fileName.data());
		std::string line;
		std::stringstream ss;
		while (std::getline(file, line))
		{
			if (line.find("include") != std::string::npos)
			{
				auto includeFile = line.substr(line.find_first_of('"') + 1, line.find_last_of('"') - line.find_first_of('"') - 1);

				size_t offset = 0;
				if ((offset = fileName.find_last_of("/ \\")) != std::string::npos)
					includeFile = std::string(fileName.substr(0, offset + 1)) + includeFile;

				ss << GetSource(includeFile);
			}
			else
				ss << line << '\n';
		}
		return ss.str();
	}

	static std::unordered_map<ShaderStage, std::string> GetShaderSources(const std::string& file_name)
	{
		std::ifstream file(file_name);
		std::unordered_map<ShaderStage, std::string> sources;
		std::unordered_map<ShaderStage, std::stringstream> ss;

		std::string line;
		int32_t index = -1;
		while (std::getline(file, line))
		{
			if (line.find('$') != std::string::npos || line.find('@') != std::string::npos)
			{
				if (line.find("type") != std::string::npos)
				{
					if (line.find("vertex") != std::string::npos)
						index = (int32_t)ShaderStage::Vertex;
					else
						index = (int32_t)ShaderStage::Fragment;
				}
				else if (line.find("include") != std::string::npos && index > -1)
				{
					auto includeFile = line.substr(line.find_first_of('"') + 1, line.find_last_of('"') - line.find_first_of('"') - 1);

					size_t offset = 0;
					if ((offset = file_name.find_last_of("/ \\")) != std::string::npos)
						includeFile = file_name.substr(0, offset + 1) + includeFile;

					ss[(ShaderStage)index] << GetSource(includeFile);
				}
			}
			else if (index > -1)
				ss[(ShaderStage)index] << line << '\n';
		}

		for (auto& [stage, stream] : ss)
			sources[stage] = stream.str();

		file.close();
		return sources;
	}

	static std::unordered_map<ShaderStage, std::string> GetShaderSources(std::stringstream& file)
	{
		std::unordered_map<ShaderStage, std::string> sources;
		std::unordered_map<ShaderStage, std::stringstream> ss;

		std::string line;
		int32_t index = -1;
		while (std::getline(file, line))
		{
			if (line.find('$') != std::string::npos || line.find('@') != std::string::npos)
			{
				if (line.find("type") != std::string::npos)
				{
					if (line.find("vertex") != std::string::npos)
						index = (int32_t)ShaderStage::Vertex;
					else
						index = (int32_t)ShaderStage::Fragment;
				}
			}
			else if (index > -1)
				ss[(ShaderStage)index] << line << '\n';
		}

		for (auto& [stage, stream] : ss)
			sources[stage] = stream.str();

		return sources;
	}

	static VkDescriptorType GetVkDescriptorType(ShaderData type)
	{
		if (ShaderDataType::GetTypeSize(type))
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		else
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	}

	static shaderc_shader_kind ShaderTypeToShadercEnum(ShaderStage type)
	{
		switch (type)
		{
		case ShaderStage::Vertex:
			return shaderc_glsl_vertex_shader;
		case ShaderStage::Fragment:
			return shaderc_glsl_fragment_shader;
		}
		ET_ASSERT_MSG_BREAK(false, "Invalid shader type: {0}", (int32_t)type);
		return shaderc_glsl_vertex_shader;
	}

	static VkShaderStageFlagBits GetVkShaderStage(ShaderStage type)
	{
		switch (type)
		{
		case ShaderStage::Vertex:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case ShaderStage::Fragment:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		}
		ET_ASSERT_MSG_BREAK(false, "Invalid shader type: {0}", (int32_t)type);
		return VK_SHADER_STAGE_VERTEX_BIT;
	}

	VulkanShader::VulkanShader(const std::string& name, const std::unordered_map<ShaderStage, std::string>& shaderSources)
		: Shader(name)
	{
		CompileShaders(shaderSources);
		SetupDescriptors(ExtractInfo(shaderSources));
	}

	VulkanShader::VulkanShader(const std::string& name, const std::unordered_map<ShaderStage, std::string>& shaderSources, const ShaderCreateInfo& createInfo)
		: Shader(name)
	{
		CompileShaders(shaderSources);
		SetupDescriptors(createInfo);
		vertexAttributes = createInfo.vertexAttributes;
		vertexBindings = createInfo.vertexBindings;
	}

	VulkanShader::~VulkanShader()
	{
		for (auto&& [stage, module] : mShaderModules)
			vkDestroyShaderModule(VulkanAPI::GetDevice(), module, nullptr);

		vkDestroyPipelineLayout(VulkanAPI::GetDevice(), mPipelineLayout, nullptr);

		if (mLayout)
			vkDestroyDescriptorSetLayout(VulkanAPI::GetDevice(), mLayout, nullptr);

		if (mPool)
			vkDestroyDescriptorPool(VulkanAPI::GetDevice(), mPool, nullptr);

	}

	void VulkanShader::SetUniform(const std::string& uniformName, const void* data)
	{
		auto it = mUniformBindings.find(uniformName);
		if (it == mUniformBindings.end())
		{
			ET_LOG_ERROR("Failed to find uniform '{0}' in shader '{1}'!", uniformName, name);
			return;
		}
		uint32_t binding = it->second;
		mUniformBuffers[binding]->SetDataMapped(VulkanAPI::GetCurrentFrame(), data);
	}
	
	void VulkanShader::SetTextures(const std::string& uniformName, const std::vector<Ref<Texture>>& textures)
	{
		if (textures.empty())
			return;

		auto it = mTextureBindings.find(uniformName);
		if (it == mTextureBindings.end())
		{
			ET_LOG_ERROR("Failed to find uniform '{0}' in shader '{1}'!", uniformName, name);
			return;
		}
		uint32_t binding = std::get<uint32_t>(it->second);

		for (size_t j = 0; j < VulkanAPI::GetSurfaceDetails().imageCount; j++)
		{
			std::vector<VkDescriptorImageInfo> imageInfos(textures.size());
			for (size_t i = 0; i < textures.size(); i++)
			{
				VulkanTexture& texture = *(VulkanTexture*)(textures[i].get());
				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.sampler = texture.GetSampler();
				imageInfo.imageView = texture.GetImage().GetImageView();

				imageInfos[i] = (imageInfo);
			}

			VkWriteDescriptorSet update{};
			update.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			update.descriptorCount = (uint32_t)imageInfos.size();
			update.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			update.dstArrayElement = 0;
			update.dstBinding = binding;
			update.dstSet = mSets[j];
			update.pImageInfo = imageInfos.data();

			vkUpdateDescriptorSets(VulkanAPI::GetDevice(), 1, &update, 0, nullptr);
		}
	}

	void VulkanShader::CompileShaders(const std::unordered_map<ShaderStage, std::string>& shaderSources)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
		options.SetOptimizationLevel(shaderc_optimization_level_performance);

		for (auto&& [stage, source] : shaderSources)
		{
			shaderc::SpvCompilationResult compileResult = compiler.CompileGlslToSpv(source, ShaderTypeToShadercEnum(stage), name.c_str(), options);
			if (compileResult.GetCompilationStatus() != shaderc_compilation_status_success)
			{
				ET_LOG_ERROR("[SHADERC]: Compilation Error: {0}", compileResult.GetErrorMessage());
				ET_ASSERT_NO_MSG_BREAK(false);
			}
			std::vector<uint32_t> byteCode(compileResult.cbegin(), compileResult.cend());

			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = byteCode.size() * 4;
			createInfo.pCode = byteCode.data();

			VkResult err = vkCreateShaderModule(VulkanAPI::GetDevice(), &createInfo, nullptr, &mShaderModules[stage]);
			ET_VK_ASSERT_MSG(err, "Failed to create shader module!");
		}
	}

	ShaderCreateInfo VulkanShader::ExtractInfo(const std::unordered_map<ShaderStage, std::string>& shaderSources)
	{
		vertexAttributes = BasicVertexAttributes;
		vertexBindings = BasicVertexBinding;
		return BasicShaderCreateInfo;
	}

	void VulkanShader::SetupDescriptors(const ShaderCreateInfo& createInfo)
	{
		pushConstant = PushConstant(createInfo.pushConstants);

		VkPushConstantRange pushConstant{};
		pushConstant.offset = 0;
		pushConstant.size = (uint32_t)this->pushConstant.size();
		pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		if (createInfo.uniformDescriptions.empty())
		{
			VkPipelineLayoutCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			createInfo.setLayoutCount = 0;
			createInfo.pSetLayouts = nullptr;
			if (pushConstant.size > 0)
			{
				createInfo.pushConstantRangeCount = 1;
				createInfo.pPushConstantRanges = &pushConstant;
			}

			VkResult err = vkCreatePipelineLayout(VulkanAPI::GetDevice(), &createInfo, nullptr, &mPipelineLayout);
			ET_VK_ASSERT_MSG(err, "Failed to create pipeline layout!");
			return;
		}
		
		std::vector<VkDescriptorPoolSize> types;
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		std::vector<std::vector<VkWriteDescriptorSet>> descriptorSets;
		std::vector<std::vector<VkDescriptorBufferInfo>> bufferInfos;

		{
			int32_t bufferCount = 0;
			for (auto& uniform : createInfo.uniformDescriptions)
				if (GetVkDescriptorType(uniform.bindingInfo) != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
					++bufferCount;

			for (uint32_t i = 0; i < VulkanAPI::GetSurfaceDetails().imageCount; i++)
			{
				descriptorSets.push_back(std::vector<VkWriteDescriptorSet>());
				bufferInfos.push_back(std::vector<VkDescriptorBufferInfo>());
				descriptorSets.back().resize(bufferCount);
				bufferInfos.back().resize(bufferCount);
			}
		}

		int32_t bufferIndex = 0;
		for (auto& uniform : createInfo.uniformDescriptions)
		{
			VkDescriptorPoolSize size{};
			size.descriptorCount = VulkanAPI::GetSurfaceDetails().imageCount;
			size.type = GetVkDescriptorType(uniform.bindingInfo);

			types.push_back(size);

			VkDescriptorSetLayoutBinding binding{};
			binding.binding = uniform.binding;
			size_t arraySize = 0;
			if ((arraySize = ShaderDataType::GetArraySize(uniform.bindingInfo)))
				binding.descriptorCount = (uint32_t)arraySize;
			else
				binding.descriptorCount = 1;
			binding.descriptorType = size.type;
			binding.pImmutableSamplers = nullptr;
			binding.stageFlags = GetVkShaderStage(uniform.stage);

			bindings.push_back(binding);

			if (size.type != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				BufferCreateInfo bufferCreateInfo{};
				bufferCreateInfo.memoryPropertyFlags = MemoryPropertyFlags_HostVisible | MemoryPropertyFlags_HostCoherent;
				bufferCreateInfo.size = ShaderDataType::GetTypeSize(uniform.bindingInfo);
				size_t arraySize = 0;
				if ((arraySize = ShaderDataType::GetArraySize(uniform.bindingInfo)))
					bufferCreateInfo.size *= arraySize;
				bufferCreateInfo.usageFlags = BufferUsageFlags_UniformBuffer;

				mUniformBuffers.emplace_back(CreateBufferArray(bufferCreateInfo, (size_t)VulkanAPI::GetSurfaceDetails().imageCount));
				auto& buf = *(mUniformBuffers.back());
				mUniformBindings[uniform.name] = (uint32_t)(mUniformBuffers.size() - 1);

				for (uint32_t i = 0; i < VulkanAPI::GetSurfaceDetails().imageCount; i++)
				{
					VkDescriptorBufferInfo info{};
					info.buffer = *((VulkanBuffer*)buf[i].get());
					info.offset = 0;
					info.range = buf.Size();
					bufferInfos[i][bufferIndex] = info;

					VkWriteDescriptorSet writeSet{};
					writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writeSet.descriptorCount = binding.descriptorCount;
					writeSet.descriptorType = size.type;
					writeSet.dstArrayElement = 0;
					writeSet.dstBinding = uniform.binding;
					writeSet.pBufferInfo = &bufferInfos[i][bufferIndex];
					descriptorSets[i][bufferIndex] = writeSet;
				}
				++bufferIndex;
			}
			else
			{
				mTextureBindings[uniform.name] = { uniform.binding, binding.descriptorCount };
			}
			
		}

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.maxSets = VulkanAPI::GetSurfaceDetails().imageCount;
		poolInfo.poolSizeCount = (uint32_t)types.size();
		poolInfo.pPoolSizes = types.data();

		VkResult err = vkCreateDescriptorPool(VulkanAPI::GetDevice(), &poolInfo, nullptr, &mPool);
		ET_VK_ASSERT_MSG(err, "Failed to create descriptor pool for shader " + name);

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = (uint32_t)bindings.size();
		layoutInfo.pBindings = bindings.data();

		err = vkCreateDescriptorSetLayout(VulkanAPI::GetDevice(), &layoutInfo, nullptr, &mLayout);
		ET_VK_ASSERT_MSG(err, "Failed to create descriptor set layout for shader " + name);

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &mLayout;
		if (pushConstant.size > 0)
		{
			pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
			pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstant;
		}

		err = vkCreatePipelineLayout(VulkanAPI::GetDevice(), &pipelineLayoutCreateInfo, nullptr, &mPipelineLayout);
		ET_VK_ASSERT_MSG(err, "Failed to create pipeline layout!");

		std::vector<VkDescriptorSetLayout> layouts(VulkanAPI::GetSurfaceDetails().imageCount, mLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = mPool;
		allocInfo.descriptorSetCount = VulkanAPI::GetSurfaceDetails().imageCount;
		allocInfo.pSetLayouts = layouts.data();

		mSets.resize(VulkanAPI::GetSurfaceDetails().imageCount);

		err = vkAllocateDescriptorSets(VulkanAPI::GetDevice(), &allocInfo, mSets.data());
		ET_VK_ASSERT_MSG(err, "Failed to allocate descriptor set for shader " + name);

		for (uint32_t i = 0; i < VulkanAPI::GetSurfaceDetails().imageCount; i++)
		{
			for (auto& write_set : descriptorSets[i])
				write_set.dstSet = mSets[i];

			vkUpdateDescriptorSets(VulkanAPI::GetDevice(), (uint32_t)descriptorSets[i].size(), descriptorSets[i].data(), 0, nullptr);
		}
	}

	std::vector<VkPipelineShaderStageCreateInfo> VulkanShader::GetShaderStages() const
	{
		std::vector<VkPipelineShaderStageCreateInfo> stages;
		for (auto&& [stage, module] : mShaderModules)
		{
			VkPipelineShaderStageCreateInfo stageInfo{};
			stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stageInfo.module = module;
			stageInfo.pName = "main";
			stageInfo.stage = GetVkShaderStage(stage);

			stages.push_back(stageInfo);
		}
		return stages;
	}

	Ref<Shader> CreateShader(const std::string& fileName)
	{
		return CreateRef<VulkanShader>(get_file_name(fileName), GetShaderSources(fileName));
	}

	Ref<Shader> CreateShader(const std::string& fileName, const ShaderCreateInfo& createInfo)
	{
		return CreateRef<VulkanShader>(get_file_name(fileName), GetShaderSources(fileName), createInfo);
	}

	Ref<Shader> CreateShader(std::stringstream& source)
	{
		// TODO: fix name
		return CreateRef<VulkanShader>("no_name", GetShaderSources(source));
	}

	Ref<Shader> CreateShader(std::stringstream& source, const ShaderCreateInfo& createInfo)
	{
		return CreateRef<VulkanShader>("et_file_name(fileName)", GetShaderSources(source), createInfo);
	}
}