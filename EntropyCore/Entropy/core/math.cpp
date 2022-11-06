#include <etpch.h>
#include <glm/gtx/matrix_decompose.hpp>

#include "math.h"

namespace et::Math
{
	bool DecomposeMatrix(const glm::mat4& transform, glm::vec3& position, glm::vec3& rotation, glm::vec3& scale)
	{
		glm::quat rot;
		glm::vec3 skew;
		glm::vec4 proj;
		if (!glm::decompose(transform, scale, rot, position, skew, proj))
			return false;
		rotation = glm::eulerAngles(rot);
		return true;
	}
}