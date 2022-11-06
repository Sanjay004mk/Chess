#include <etpch.h>

#include "push_constant.h"

namespace et
{
	PushConstant::PushConstant(const std::vector<ShaderData> attributes)
	{
		memset(data, 0, sizeof(data));
		pushConstantOffsets.resize(attributes.size(), 0);
		if (!attributes.empty())
		{
			Size += ShaderDataType::GetTypeSize(attributes[0]);
			for (size_t i = 1; i < attributes.size(); i++)
			{
				for (int32_t j = int32_t(i - 1); j >= 0; j--)
					pushConstantOffsets[i] += ShaderDataType::GetTypeSize(attributes[j]);

				Size += ShaderDataType::GetTypeSize(attributes[i]);
			}
		}

	}
}