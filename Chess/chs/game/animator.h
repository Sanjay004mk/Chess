#pragma once
#include "board.h"

namespace chs
{
	struct KeyFrame
	{
		glm::vec2 position;
		uint32_t frame = 0;

		bool operator==(const KeyFrame& other) const
		{
			return position == other.position && frame == other.frame;
		}
	};

	struct AnimationInfo
	{
		AnimationInfo(const glm::vec2& from, const glm::vec2& to, float length);

		std::vector<KeyFrame> keyFrames;
		uint32_t fps = 0;

		std::pair<KeyFrame&, KeyFrame&> find(uint32_t frame);

		bool operator==(const AnimationInfo& other) const
		{
			return keyFrames == other.keyFrames && fps == other.fps;
		}

	private:
		friend class Animator;
		bool animating = false;
		float currentFrame = 0.f;
		glm::vec2 currentFrameOffset = {};
	};

	class Animator
	{
	public:
		static void OnUpdate(et::TimeStep ts);
		static size_t Play(const AnimationInfo& animation);

		static bool Animate(glm::vec2& position, size_t animation_id);

	private:
		static std::vector<AnimationInfo> animations;
	};
}