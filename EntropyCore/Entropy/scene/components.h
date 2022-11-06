#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <string>

#include "Entropy/EntropyUtils.h"

namespace et
{
	struct Vertex
	{
		alignas(8)  glm::vec2 position;
		alignas(8)  glm::vec2 uv;
		alignas(16) glm::vec3 color;

		bool operator==(const Vertex& other) const { return position == other.position && uv == other.uv && color == other.color; }
	};

	struct Quad
	{
		glm::vec2 position = { 0.f, 0.f };
		glm::vec2 size = { 1.f, 1.f };
		glm::vec2 uvs[4] = {};
		glm::vec3 color = { 1.f, 0.f, 1.f };
	};
}

namespace std
{
	template<>
	struct hash<et::Vertex>
	{
		size_t operator()(et::Vertex const& v) const
		{
			size_t seed = std::hash<glm::vec2>()(v.position);
			seed ^= std::hash<glm::vec2>()(v.uv) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= std::hash<glm::vec3>()(v.color) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			return seed;
		}
	};

}