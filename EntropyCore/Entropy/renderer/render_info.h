#pragma once

namespace et
{
	enum class DataType
	{
		Unknown = 0,
		Bool, Uint8, Int8, Char,
		Uint16, Int16, Uint32, Int32, Float,
		Uint64, Int64, Double, Vec2,
		Vec3,
		Vec4,
		Mat3,
		Mat4,
	};
#pragma region Texture

	using TextureUsageFlags = uint32_t;
	enum : uint32_t
	{
		TextureUsageFlags_TransferSrc					= 0x1,
		TextureUsageFlags_TransferDst					= 0x2,
		TextureUsageFlags_Sampled						= 0x4,
		TextureUsageFlags_Storage						= 0x8,
		TextureUsageFlags_ColorAttachment				= 0x10,
		TextureUsageFlags_DepthStencilAttachment		= 0x20,
		TextureUsageFlags_TransientAttachment			= 0x40,
		TextureUsageFlags_InputAttachment				= 0x80,
		TextureUsageFlags_FragmentShadingRateAttachment = 0x100,
		TextureUsageFlags_FragmentDensityMap			= 0x200,
	};

	enum class TextureFormat {
		Undefined = 0,
		R8Unorm = 9, R8Snorm = 10, R8Uint = 13, R8Sint = 14, R8Srgb = 15,
		R8G8Unorm = 16, R8G8Snorm = 17, R8G8Uint = 20, R8G8Sint = 21, R8G8Srgb = 22,
		R8G8B8Unorm = 23, R8G8B8Snorm = 24, R8G8B8Uint = 27, R8G8B8Sint = 28, R8G8B8Srgb = 29,
		B8G8R8Unorm = 30, B8G8R8Snorm = 31, B8G8R8Uint = 34, B8G8R8Sint = 35, B8G8R8Srgb = 36,
		R8G8B8A8Unorm = 37, R8G8B8A8Snorm = 38, R8G8B8A8Uint = 41, R8G8B8A8Sint = 42, R8G8B8A8Srgb = 43,
		B8G8R8A8Unorm = 44, B8G8R8A8Snorm = 45, B8G8R8A8Uint = 48, B8G8R8A8Sint = 49, B8G8R8A8Srgb = 50,
		R32Uint = 98, R32Sint = 99, R32SFloat = 100,
		R32G32B32A32SFloat = 109,
		D16Unorm = 124, D32Float = 126, S8Uint = 127, D16UnormS8Uint = 128, D24UnormS8Uint = 129,
	};

	enum class FilterType 
	{
		Nearest = 0,
		Linear = 1,
		Cubic = 1000015000 
	};

	enum class SamplerAddressMode 
	{ 
		Repeat = 0,
		MirroredRepeat = 1,
		ClampToEdge = 2,
		ClampToBorder = 3,
		MirrorClampToEdge = 4 
	};

	enum class SampleCount
	{
		Num1	= 1,
		Num2	= 2,
		Num4	= 4,
		Num8	= 8,
		Num16	= 10,
		Num32	= 20,
		Num64	= 40
	};

	enum class ImageLayout
	{
		Undefined = 0,
		General = 1,
		ColorAttachment = 2,
		DepthStencilAttachment = 3,
		DepthStencilReadOnly = 4,
		ShaderReadOnly = 5,
		TransferSrc = 6,
		TransferDst = 7,
		PreInitialized = 8,
		DepthReadOnlyStencilAttachment = 1000117000,
		DepthAttachmentStencilReadOnly = 1000117001,
		DepthAttachment = 1000241000,
		DepthReadOnly = 1000241001,
		StencilAttachment = 1000241002,
		StencilReadOnly = 1000241003,
		ReadOnly = 1000314000,
		Attachment = 1000314001,
		PresentSrc = 1000001002
	};

	inline int32_t GetNumChannels(TextureFormat format)
	{
		int32_t i = (int32_t)format;
		if (i > 36)
			return 4;
		else if (i > 22)
			return 3;
		else if (i > 15)
			return 2;
		else
			return 1;
	}

	inline TextureFormat ToTextureFormat(uint32_t channels)
	{
		if (channels == 4)
			return TextureFormat::R8G8B8A8Unorm;
		else if (channels == 3)
			return TextureFormat::R8G8B8Unorm;
		else if (channels == 2)
			return TextureFormat::R8G8Unorm;
		else if (channels == 1)
			return TextureFormat::R8Unorm;
		else
		{
			ET_LOG_WARN("Potentially wrong number of channels {0} Line: {1} File: {2}", channels, __LINE__, __FILE__);
			return TextureFormat::Undefined;
		}
	}

#pragma endregion

#pragma region Shader

	using ShaderData = uint64_t;
	namespace ShaderDataType
	{
		inline ShaderData SetArraySize(size_t size)
		{
			return (size) & 0x0000ffff;
		}

		inline size_t GetArraySize(ShaderData data)
		{
			return (data & 0x0000ffff);
		}

		inline size_t GetTypeSize(ShaderData data)
		{
			return (data & 0xffff0000) >> 16;
		}

		inline constexpr ShaderData SetTypeSize(size_t size)
		{
			return (size << 16) & 0xffff0000;
		}

		inline ShaderData SetTypeSize(const std::vector<ShaderData>& types)
		{
			size_t size = 0;
			for (auto type : types)
				size += GetTypeSize(type);

			return SetTypeSize(size);
		}

		inline ShaderData SetTypeSizePadded(const std::vector<ShaderData>& types)
		{
			size_t size = 0;
			for (auto type : types)
				size += ShaderDataType::GetTypeSize(type);

			return SetTypeSize(size + (size % 64));
		}

		enum : ShaderData
		{
			Invalid = (~0),
			ImageSampler = 0,
			Int = SetTypeSize(4),
			Uint = SetTypeSize(4),
			Float = SetTypeSize(4),
			Double = SetTypeSize(8),
			Bool = SetTypeSize(1),
			Vec2f = SetTypeSize(8),
			Vec3f = SetTypeSize(16),
			Vec4f = SetTypeSize(16),
			Mat4f = SetTypeSize(64),
		};
	}

	enum class ShaderStage
	{
		Vertex = 0,
		Fragment = 1,
	};

	enum class VertexInputRate
	{
		PerVertex = 0,
		PerInstance = 1,
	};

	struct VertexInputBinding
	{
		uint32_t binding;
		uint32_t stride;
		VertexInputRate inputRate = VertexInputRate::PerVertex;
	};

	struct VertexInputAttribute
	{
		uint32_t location;
		uint32_t binding;
		DataType format;
		uint32_t offset;
	};

	struct UniformDescription
	{
		ShaderStage stage;
		uint32_t binding;
		ShaderData bindingInfo;
		std::string name;
	};

#pragma endregion

#pragma region Renderpass
	enum class AttachmentLoadOp
	{
		Load = 0,
		Clear = 1,
		DontCare = 2,
		None = 1000400000
	};

	enum class AttachmentStoreOp
	{
		Store = 0,
		DontCare = 1,
		None = 1000301000
	};

#pragma endregion

#pragma region Pipeline

	struct Scissors
	{
		glm::vec2 offset = glm::vec2(0.f);
		glm::vec2 extent = glm::vec2(0.f);

		bool operator==(const Scissors& other) const
		{
			return offset == other.offset && extent == other.extent;
		}
	};
	struct Viewport
	{
		float x = 0.f, y = 0.f;
		float width = 0.f, height = 0.f;
		float min_depth = 0.f, max_depth = 0.f;

		bool operator==(const Viewport& other) const
		{
			return 
				x == other.x && y == other.y && 
				width == other.width &&  height == other.height && 
				min_depth == other.min_depth && max_depth == other.max_depth;
		}
	};

	enum class CullMode
	{ 
		None = 0,
		FrontFace, 
		BackFace, 
		FrontAndBack 
	};

	enum class FrontFaceType 
	{ 
		CouterClockWise = 0,
		ClockWise 
	};

	enum class CompareOp 
	{ 
		Never = 0,
		Less, 
		Equal, 
		LEqual, 
		Greater, 
		NotEqual, 
		GEqual, 
		Always 
	};

	enum class PolygonMode 
	{ 
		Fill = 0,
		Line, 
		Point 
	};

	enum class Topology
	{
		PointList = 0,
		LineList = 1,
		LineStrip = 2,
		TriangleList = 3,
		TriangleStrip = 4,
		TriangleFan = 5,
		LineListWithAdjacency = 6,
		LineStripWithAdjacency = 7,
		TriangleListWithAdjacency = 8,
		TriangleStripWithAdjacency = 9,
		PatchList = 10,
	};

	enum class StencilOp { 
		Keep = 0,
		Zero = 1,
		Replace = 2, 
		IncrementAndClamp = 3, 
		DecrementAndClamp = 4, 
		Invert = 5,
		IncrementAndWrap = 6,
		DecrementAndWrap = 7 
	};

	enum class PipelineBindPoint
	{
		Graphics = 0,
		Compute = 1,
		RayTracing = 1000165000
	};

	enum class PipelineStage
	{
		None					= 0x0,
		TopOfPipe				= 0x1,
		DrawIndirect			= 0x2,
		VertexInput				= 0x4,
		VertexShader			= 0x8,
		TessellationControl		= 0x10,
		TesselalationEval		= 0x20,
		GeometryShader			= 0x40,
		FragmentShader			= 0x80,
		EarlyFragmentTests		= 0x100,
		LateFragmentTests		= 0x200,
		ColorAttachmentOutput	= 0x400,
		ComputeShader			= 0x800,
		Transfer				= 0x1000,
		BottomOfPipe			= 0x2000,
		Host					= 0x4000,
		AllGraphics				= 0x8000,
		AllCommands				= 0x10000,
		RayTracingShader		= 0x200000,
		TaskShaderNv			= 0x80000,
		MeshShaderNv			= 0x100000,
	};

	enum class AccessFlags
	{
		None						= 0x0,
		IndirectCommandRead			= 0x1,
		IndexRead					= 0x2,
		VertexAttributeRead			= 0x4,
		UniformRead					= 0x8,
		InputAttachmentRead			= 0x10,
		ShaderRead					= 0x20,
		ShaderWrite					= 0x40,
		ColorAttachmentRead			= 0x80,
		ColorAttachmentWrite		= 0x100,
		DepthStencilAttachmentRead	= 0x200,
		DepthStencilAttachmentWrite = 0x400,
		TrasferRead					= 0x800,
		TransferWrite				= 0x1000,
	};
#pragma endregion

#pragma region Buffer

	using BufferUsageFlags = uint32_t;
	enum : uint32_t
	{
		BufferUsageFlags_TransferSrc = 0x1,
		BufferUsageFlags_TransferDst = 0x2,
		BufferUsageFlags_UniformTexelBuffer = 0x4,
		BufferUsageFlags_StorageTexelBuffer = 0x8,
		BufferUsageFlags_UniformBuffer = 0x10,
		BufferUsageFlags_StorageBuffer = 0x20,
		BufferUsageFlags_IndexBuffer = 0x40,
		BufferUsageFlags_VertexBuffer = 0x80,
		BufferUsageFlags_IndirectBuffer = 0x100,
	};

	using MemoryPropertyFlags = uint32_t;
	enum : uint32_t
	{
		MemoryPropertyFlags_DeviceLocal = 0x1,
		MemoryPropertyFlags_HostVisible = 0x2,
		MemoryPropertyFlags_HostCoherent = 0x4,
		MemoryPropertyFlags_HostCached = 0x8,
		MemoryPropertyFlags_LazilyAllocated = 0x10,
		MemoryPropertyFlags_Protected = 0x20,
		MemoryPropertyFlags_DeviceCoherent = 0x40,
		MemoryPropertyFlags_DeviceUncached = 0x80,
	};
#pragma endregion
}