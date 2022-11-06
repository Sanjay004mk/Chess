#include <etpch.h>
#include <filesystem>
#include <shaderc/shaderc.hpp>

#include "Entropy/scene/components.h"
#include "vkapi.h"

#if 0
namespace et
{
	static ShaderData StringToDataType(std::string& str, bool log_error = false)
	{
		ShaderData type = ShaderDataType::Invalid;
		trim(str);

		if (str.find("[") != std::string::npos)
			type |= ShaderDataType::Array;

		if (str == "int")
			type |= ShaderDataType::Int;
		else if (str == "float")
			type |= ShaderDataType::Float;
		else if (str == "unsigned int")
			type |= ShaderDataType::Uint;
		else if (str == "double")
			type |= ShaderDataType::Double;
		else if (str == "vec2")
			type |= ShaderDataType::Vec2 | ShaderDataType::Float;
		else if (str == "vec3")
			type |= ShaderDataType::Vec3 | ShaderDataType::Float;
		else if (str == "vec4")
			type |= ShaderDataType::Vec4 | ShaderDataType::Float;
		else if (str == "bool")
			type |= ShaderDataType::Bool;
		else if (str == "sampler2D")
			type |= ShaderDataType::Sampler2d;
		else if (str == "mat4")
			type |= ShaderDataType::Mat4 | ShaderDataType::Float;
		else if (log_error)
			ET_LOG_ERROR("Unable to deduce data type: {0}", str);
		return type;
	}

	Shader::Shader(const char* file_name, const std::string& _name, VertexBindFn bind_fn, VertexAttrFn attr_fn)
		: name(_name.empty() ? get_file_name(file_name) : _name), get_vertex_attr_desc(attr_fn), get_vertex_bind_desc(bind_fn)
	{
		Init(file_name);
	}

	Shader::Shader(const char* file_name, VkRenderPass render_pass, const ShaderCreateInfo& create_info, const std::string& _name, VertexBindFn bind_fn, VertexAttrFn attr_fn)
		: name(_name.empty() ? get_file_name(file_name) : _name), get_vertex_attr_desc(attr_fn), get_vertex_bind_desc(bind_fn)
	{
		Init(file_name);
		CreateShaderModule(render_pass, create_info);
	}

	void Shader::Init(const char* file_name)
	{
		auto sources = GetShaderSources(file_name);

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
		options.SetOptimizationLevel(shaderc_optimization_level_performance);

		for (auto&& [stage, source] : sources)
		{
			shaderc::SpvCompilationResult compileResult = compiler.CompileGlslToSpv(source, ShaderTypeToShadercEnum(stage), file_name, options);
			if (compileResult.GetCompilationStatus() != shaderc_compilation_status_success)
			{
				ET_LOG_ERROR("{0}", compileResult.GetErrorMessage());
			}
			shaderSPIRV[stage] = std::vector<uint32_t>(compileResult.cbegin(), compileResult.cend());
			ExtractAttributesAndUniforms(stage, source);
		}
		SetupDescriptors();
	}

	void Shader::CreateShaderModule(VkRenderPass render_pass, const ShaderCreateInfo& create_info)
	{
		VkShaderModule shader_modules[2];
		VkPipelineShaderStageCreateInfo shader_stages[2];

		for (auto&& [stage, byte_code] : shaderSPIRV)
		{
			VkShaderModuleCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			info.codeSize = byte_code.size() * sizeof(uint32_t);
			info.pCode = byte_code.data();
			VkResult err = vkCreateShaderModule(VulkanAPI::GetDevice(), &info, nullptr, &shader_modules[(size_t)stage]);
			ET_VK_ASSERT_MSG(err, "Failed to create shader module!");

			shader_stages[(size_t)stage].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shader_stages[(size_t)stage].module = shader_modules[(uint32_t)stage];
			shader_stages[(size_t)stage].pName = "main";
			shader_stages[(size_t)stage].stage = GetVkShaderStage(stage);
			shader_stages[(size_t)stage].flags = 0;
			shader_stages[(size_t)stage].pNext = nullptr;
			shader_stages[(size_t)stage].pSpecializationInfo = nullptr;
		}	
		auto vert_attr_desc = get_vertex_attr_desc();
		auto vert_bind_desc = get_vertex_bind_desc();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &vert_bind_desc;
		vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)vert_attr_desc.size();
		vertexInputInfo.pVertexAttributeDescriptions = vert_attr_desc.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.primitiveRestartEnable = VK_FALSE;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		VkRect2D scissors = GetVkScissors(create_info.scissors);
		VkViewport viewport = GetVkViewport(create_info.viewport);

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.scissorCount = 1;
		viewportState.viewportCount = 1;
		viewportState.pScissors = &scissors;
		viewportState.pViewports = &viewport;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorAttach{};
		colorAttach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorAttach.blendEnable = create_info.blend_enable ? VK_TRUE : VK_FALSE;
		colorAttach.colorBlendOp = VK_BLEND_OP_ADD;
		colorAttach.alphaBlendOp = VK_BLEND_OP_ADD;
		colorAttach.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorAttach.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorAttach.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorAttach.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;

		VkPipelineColorBlendStateCreateInfo colorBlend{};
		colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlend.logicOpEnable = VK_FALSE;
		colorBlend.logicOp = VK_LOGIC_OP_COPY;
		colorBlend.attachmentCount = 1;
		colorBlend.pAttachments = &colorAttach;
		colorBlend.blendConstants[0] = 0.0f;
		colorBlend.blendConstants[1] = 0.0f;
		colorBlend.blendConstants[2] = 0.0f;
		colorBlend.blendConstants[3] = 0.0f;

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthWriteEnable = create_info.depth_enable ? VK_TRUE : VK_FALSE;
		depthStencil.depthTestEnable = depthStencil.depthWriteEnable;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.depthCompareOp = (VkCompareOp)create_info.depth_comapre_op;
		depthStencil.maxDepthBounds = create_info.depth_max_bound;
		depthStencil.minDepthBounds = create_info.depth_min_bound;
		depthStencil.stencilTestEnable = create_info.stencil_enable ? VK_TRUE : VK_FALSE;
		depthStencil.back.compareOp = (VkCompareOp)create_info.stencil_compare_op;
		depthStencil.back.depthFailOp = (VkStencilOp)create_info.stencil_depth_fail_op;
		depthStencil.back.failOp = (VkStencilOp)create_info.stencil_fail_op;
		depthStencil.back.passOp = (VkStencilOp)create_info.stencil_pass_op;
		depthStencil.back.compareMask = create_info.compare_mask;
		depthStencil.back.writeMask = create_info.write_mask;
		depthStencil.back.reference = create_info.reference;
		depthStencil.front = depthStencil.back;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.cullMode = (VkCullModeFlags)create_info.cull_mode;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasClamp = rasterizer.depthBiasConstantFactor = rasterizer.depthBiasSlopeFactor = 0.f;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.frontFace = (VkFrontFace)create_info.front_face;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.lineWidth = create_info.line_width;
		rasterizer.polygonMode = (VkPolygonMode)create_info.polygon_mode;

		VkGraphicsPipelineCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		createInfo.layout = pipeline_layout;
		createInfo.pColorBlendState = &colorBlend;
		createInfo.pDepthStencilState = &depthStencil;
		createInfo.pInputAssemblyState = &inputAssembly;
		createInfo.pMultisampleState = &multisampling;
		createInfo.pRasterizationState = &rasterizer;
		createInfo.stageCount = 2;
		createInfo.pStages = shader_stages;
		createInfo.pVertexInputState = &vertexInputInfo;
		createInfo.pViewportState = &viewportState;
		createInfo.renderPass = render_pass;
		createInfo.subpass = 0;
		createInfo.basePipelineHandle = VK_NULL_HANDLE;

		VkResult err = vkCreateGraphicsPipelines(VulkanAPI::GetDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline);
		ET_VK_ASSERT_MSG(err, "Failed to create graphics pipeline for shader " + name);

		for (int32_t i = 0; i < (int32_t)(sizeof(shader_modules) / sizeof(VkShaderModule)); i++)
			vkDestroyShaderModule(VulkanAPI::GetDevice(), shader_modules[i], nullptr);
	}

	void Shader::SetupDescriptors()
	{
		if (uniforms.empty())
		{
			VkPipelineLayoutCreateInfo pipeline_layout_info{};
			pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipeline_layout_info.setLayoutCount = 0;
			pipeline_layout_info.pSetLayouts = nullptr;
			pipeline_layout_info.pushConstantRangeCount = 0;
			pipeline_layout_info.pPushConstantRanges = nullptr;

			VkResult res = vkCreatePipelineLayout(VulkanAPI::GetDevice(), &pipeline_layout_info, nullptr, &pipeline_layout);
			ET_VK_ASSERT_MSG(res, "Failed to create pipeline layout for shader " + name);
			return;
		}
		std::vector<VkDescriptorPoolSize> types;
		std::vector<VkDescriptorSetLayoutBinding> bindings;

		std::vector<std::vector<VkWriteDescriptorSet>> descriptor_sets;
		std::vector<std::vector<VkDescriptorBufferInfo>> buffer_info;

		{
			int32_t count = 0;
			for (auto&& [stage, descriptors] : uniforms)
				for (auto& descriptor : descriptors)
				{
					if (GetVkDescriptorType(descriptor.second.second[0]) != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
						++count;
				}
			for (uint32_t i = 0; i < VulkanAPI::GetSurfaceDetails().imageCount; i++)
			{
				descriptor_sets.push_back(std::vector<VkWriteDescriptorSet>());
				buffer_info.push_back(std::vector<VkDescriptorBufferInfo>());
				descriptor_sets.back().resize(count);
				buffer_info.back().resize(count);
			}
		}

		int32_t index = 0;
		for (auto&& [stage, descriptors] : uniforms)
		for (auto& descriptor : descriptors)
		{
			VkDescriptorPoolSize size{};
			size.descriptorCount = VulkanAPI::GetSurfaceDetails().imageCount;
			size.type = GetVkDescriptorType(descriptor.second.second[0]);
			
			types.push_back(size);

			VkDescriptorSetLayoutBinding binding{};
			binding.binding = descriptor.first;
			binding.descriptorCount = 1;
			binding.descriptorType = size.type;
			binding.pImmutableSamplers = nullptr;
			binding.stageFlags = GetVkShaderStage(stage);
					
			bindings.push_back(binding);

			if (size.type != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				buffers.emplace_back(new BufferArray((size_t)VulkanAPI::GetSurfaceDetails().imageCount, GetTypeSize(descriptor.second.second), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
				auto& buf = *(buffers.back());
				buffer_indices[descriptor.second.first] = (uint32_t)(buffers.size() - 1);
						
				for (uint32_t i = 0; i < VulkanAPI::GetSurfaceDetails().imageCount; i++)
				{
					VkDescriptorBufferInfo info{};
					info.buffer = buf[i];
					info.offset = 0;
					info.range = buf.Size();
					buffer_info[i][index] = info;

					VkWriteDescriptorSet write_set{};
					write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					write_set.descriptorCount = 1;
					write_set.descriptorType = size.type;
					write_set.dstArrayElement = 0;
					write_set.dstBinding = descriptor.first;
					write_set.pBufferInfo = &buffer_info[i][index];
					descriptor_sets[i][index] = write_set;
				}
				++index;
			}
			else
			{
				if (descriptor.second.second[0] & ShaderDataType::Array)
					bindings.back().descriptorCount = (uint32_t)ShaderDataType::GetArraySize(descriptor.second.second[0]);
			}
		}

		VkDescriptorPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.maxSets = VulkanAPI::GetSurfaceDetails().imageCount;
		pool_info.poolSizeCount = (uint32_t)types.size();
		pool_info.pPoolSizes = types.data();

		VkResult err = vkCreateDescriptorPool(VulkanAPI::GetDevice(), &pool_info, nullptr, &pool);
		ET_VK_ASSERT_MSG(err, "Failed to create descriptor pool for shader " + name);

		VkDescriptorSetLayoutCreateInfo layout_info{};
		layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.bindingCount = (uint32_t)bindings.size();
		layout_info.pBindings = bindings.data();

		err = vkCreateDescriptorSetLayout(VulkanAPI::GetDevice(), &layout_info, nullptr, &layout);
		ET_VK_ASSERT_MSG(err, "Failed to create descriptor set layout for shader " + name);

		VkPipelineLayoutCreateInfo pipeline_layout_info{};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount = 1;
		pipeline_layout_info.pSetLayouts = &layout;
		
		err = vkCreatePipelineLayout(VulkanAPI::GetDevice(), &pipeline_layout_info, nullptr, &pipeline_layout);
		ET_VK_ASSERT_MSG(err, "Failed to create pipeline layout for shader " + name);

		std::vector<VkDescriptorSetLayout> layouts(VulkanAPI::GetSurfaceDetails().imageCount, layout);
		VkDescriptorSetAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = pool;
		alloc_info.descriptorSetCount = VulkanAPI::GetSurfaceDetails().imageCount;
		alloc_info.pSetLayouts = layouts.data();

		set.resize(VulkanAPI::GetSurfaceDetails().imageCount);

		err = vkAllocateDescriptorSets(VulkanAPI::GetDevice(), &alloc_info, set.data());
		ET_VK_ASSERT_MSG(err, "Failed to allocate descriptor set for shader " + name);

		for (uint32_t i = 0; i < VulkanAPI::GetSurfaceDetails().imageCount; i++)
		{
			for (auto& write_set : descriptor_sets[i])
				write_set.dstSet = set[i];

			vkUpdateDescriptorSets(VulkanAPI::GetDevice(), (uint32_t)descriptor_sets[i].size(), descriptor_sets[i].data(), 0, nullptr);
		}			
	}

	VkVertexInputBindingDescription Shader::GetVertexBindDesc()
	{
		VkVertexInputBindingDescription vertex_bind_desc;
		vertex_bind_desc.binding = 0;
		vertex_bind_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		vertex_bind_desc.stride = sizeof(Vertex);
		return vertex_bind_desc;
	}

	std::vector<VkVertexInputAttributeDescription> Shader::GetVertexAttrDesc()
	{
		std::vector<VkVertexInputAttributeDescription> res(4);
		res[0].binding = 0;
		res[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		res[0].location = 0;
		res[0].offset = offsetof(Vertex, position);

		res[1].binding = 0;
		res[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		res[1].location = 1;
		res[1].offset = offsetof(Vertex, normal);

		res[2].binding = 0;
		res[2].format = VK_FORMAT_R32G32_SFLOAT;
		res[2].location = 2;
		res[2].offset = offsetof(Vertex, uv);

		res[3].binding = 0;
		res[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		res[3].location = 3;
		res[3].offset = offsetof(Vertex, color);

		return res;
	}

	void Shader::ExtractAttributesAndUniforms(ShaderType stage, const std::string& shader_source)
	{
		auto GetNextWord = [](const std::string& str, int32_t& i)
		{
			std::vector<char> char_buffer;
			while (std::isspace(str[i]))
				++i;

			while (std::isalnum(str[i]) || str[i] == '_')
				char_buffer.push_back(str[i++]);

			char_buffer.push_back(0);
			
			return std::string(char_buffer.data());
		};

		auto SeekNextChar = [](const std::string& s, char ch, int32_t& i)
		{
			char c = 0;
			do
			{
				c = s[i++];
			} while (c != ch);
		};

		auto SeekNearestChar = [](char& c, const std::string& s, char c1, char c2, int32_t& i)
		{
			do
			{
				c = s[i++];
			} while (c != c1 && c != c2);
		};

		std::string s = shader_source;

		// trim any comments
		size_t comment_start = s.find("//");
		while (comment_start != std::string::npos)
		{
			char ch = 0;
			int32_t i = (int32_t)comment_start + 2;
			do
			{
				ch = s[i++];
			} while (ch != '\n');
			s = s.substr(0, comment_start) + s.substr(i);
			comment_start = s.find("//");
		}
		comment_start = s.find("/*");
		while (comment_start != std::string::npos)
		{
			char ch = 0;
			int32_t i = (int32_t)comment_start + 3;
			do
			{
				ch = s[i++];
			} while (ch != '*' && s[i] != '/');
			s = s.substr(0, comment_start) + s.substr(i + 1);
			comment_start = s.find("/*");
		}
		s.erase(std::remove(s.begin(), s.end(), '\n'), s.cend());
		size_t pos = s.find("layout");
		while (pos != std::string::npos)
		{
			s = s.substr(pos + 6);  // set position to next letter after 'layout'

			char c = 0;
			int32_t i = 0;
			std::pair<std::string, std::vector<ShaderData>>* data = nullptr;
			std::string qualifier{}, type{};
						
			SeekNextChar(s, '=', i);
			int32_t index = std::stoi(GetNextWord(s, i));
			SeekNextChar(s, ')', i);

			qualifier = GetNextWord(s, i);

			if (qualifier == "out")
			{
				pos = s.find("layout");
				continue;
			}
			else if (qualifier == "uniform")
				data = &uniforms[stage][index];
			else
				data = &attributes[stage][index];

			ShaderData data_type = StringToDataType(GetNextWord(s, i));
			if (data_type)
			{
				std::string name = GetNextWord(s, i);
				*data = std::make_pair(name, std::vector<ShaderData>(1, data_type));
				if (s[i] == '[')
				{
					data->second[0] |= ShaderDataType::Array;
					data->second[0] |= ShaderDataType::SetArraySize((size_t)std::stoi(GetNextWord(s, i = i + 1)));
					i++;
				}
			}
			else
			{
				std::vector<ShaderData> temp;
				SeekNextChar(s, '{', i);
				char ch = 0;
				while (ch != '}')
				{
					while (std::isspace(s[i]))
						i++;
					if (s[i] == '}')
					{
						i++;
						*data = std::make_pair(GetNextWord(s, i), temp);
						i++;
						break;
					}
					ShaderData type = StringToDataType(GetNextWord(s, i));
					temp.push_back(type);
					GetNextWord(s, i);
					ch = s[i];
					while (ch != ';' && ch != '}')
					{
						if (ch == '[')
						{
							temp.back() |= ShaderDataType::Array;
							temp.back() |= ShaderDataType::SetArraySize((size_t)std::stoi(GetNextWord(s, i = i + 1)));
							i++;
						}
						if (s[i] == ',')
						{
							temp.push_back(type);
							GetNextWord(s, i = i + 1);
						}						
						ch = s[i];
					}
					i++;
				}
			}
			pos = s.find("layout");
		}
	}

	Shader::~Shader()
	{
		DestroyPipeline();
	}

	void Shader::ResetPipeline(VkRenderPass render_pass, const ShaderCreateInfo& create_info)
	{
		DestroyPipeline();
		SetupDescriptors();
		CreateShaderModule(render_pass, create_info);
	}

	void Shader::DestroyPipeline()
	{
		VulkanAPI::DeviceWaitIdle();
		vkDestroyDescriptorPool(VulkanAPI::GetDevice(), pool, nullptr);
		vkDestroyDescriptorSetLayout(VulkanAPI::GetDevice(), layout, nullptr);
		vkDestroyPipelineLayout(VulkanAPI::GetDevice(), pipeline_layout, nullptr);
		vkDestroyPipeline(VulkanAPI::GetDevice(), pipeline, nullptr);

		for (auto& b : buffers)
			delete b;

		buffers.clear();
	}

	void Shader::SetUniform(const std::string& uniform_name, const void* data)
	{
		auto& b = buffer_indices.find(uniform_name);
		if (b == buffer_indices.end())
		{
			ET_LOG_ERROR("unable to find uniform {0} in shader {1}", uniform_name, name);
			return;
		}
		int32_t i = (int32_t)b->second;
		buffers[i]->SetData(VulkanAPI::GetCurrentFrame(), data, buffers[i]->Size());
	}
}
#else

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
			std::vector<VkDescriptorImageInfo> imageInfos(std::get<size_t>(it->second));
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

#endif