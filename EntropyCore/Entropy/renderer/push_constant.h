#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Entropy/EntropyUtils.h"
#include "render_info.h"

namespace et
{
	class PushConstant
	{
	public:
		PushConstant() = default;
		PushConstant(const std::vector<ShaderData> attributes);		
		~PushConstant() = default;

		template <typename T>
		void Set(const T& dataValue, size_t attributePosition)
		{
			memcpy(&data[pushConstantOffsets[attributePosition]], &dataValue, sizeof(T));
		}

		void* GetData() { return data; }
		size_t size() const { return Size; }

	private:
		std::vector<size_t> pushConstantOffsets;
		size_t Size = 0;
		uint8_t data[128];
	};
}