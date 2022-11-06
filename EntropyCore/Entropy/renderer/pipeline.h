#pragma once
#include "Entropy/EntropyUtils.h"
#include "render_info.h"

namespace et
{
	class Shader;
	class Renderpass;

	struct PipelineCreateInfo
	{
		Ref<Renderpass> renderpass;
		Ref<Shader> shader;
		Viewport viewport;
		Scissors scissors;
		bool dynamicViewport = false;
		size_t colorAttachmentCount = 2;
		SampleCount sampleCount = SampleCount::Num1;
		bool colorBlendEnable = false;
		bool depthEnable = true;
		CompareOp depthCompareOp = CompareOp::Less;
		float depthMaxBound = 1.f, depthMinBound = 0.f;
		bool stencilEnable = true;
		CompareOp stencilCompareOp = CompareOp::Always;
		StencilOp stencilDepthFailOp = StencilOp::Replace, stencilFailOp = StencilOp::Replace, stencilPassOp = StencilOp::Replace;
		uint32_t stencilCompareMask = 0xff, stencilWriteMask = 0xff, stencilReference = 1;
		CullMode cullMode = CullMode::None;
		FrontFaceType frontFace = FrontFaceType::ClockWise;
		PolygonMode polygonMode = PolygonMode::Fill;
		Topology topology = Topology::TriangleList;
		float lineWidth = 1.f;
		uint32_t subpass = 0;
	};

	class Pipeline
	{
	public:
		Pipeline() = default;
		virtual ~Pipeline() {}

		virtual void SetDynamicViewport(const Viewport& viewport) = 0;

		virtual void Bind(PipelineBindPoint bindPoint = PipelineBindPoint::Graphics) = 0;
		virtual void UnBind() {}

		static const PipelineCreateInfo BasicPipelineCreateInfo;
	};

	Ref<Pipeline> CreatePipeline(const PipelineCreateInfo& createInfo);
}