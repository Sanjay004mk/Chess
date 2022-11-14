#include "animator.h"

namespace chs
{
	std::vector<AnimationInfo> Animator::animations;

	AnimationInfo::AnimationInfo(const glm::vec2& from, const glm::vec2& to, float length)
		: fps(30)
	{
		keyFrames.push_back({ from, 0 });
		keyFrames.push_back({ to, (uint32_t)(29 * length) });
	}

	std::pair<KeyFrame&, KeyFrame&> AnimationInfo::find(uint32_t frame)
	{
		size_t index0 = keyFrames.size() - 1, index1 = keyFrames.size() - 1;
		for (size_t i = 0; i < keyFrames.size(); i++)
		{
			if (keyFrames[i].frame == frame)
			{
				index0 = index1 = i;
				break;
			}
			else if (keyFrames[i].frame > frame)
			{
				if (i != 0)
				{
					ET_DEBUG_ASSERT(keyFrames[i - 1].frame < frame);
					index0 = i - 1;
				}
				else
					index0 = i;
				index1 = i;
				break;
			}
		}
		return { keyFrames[index0], keyFrames[index1] };
	}

	template <typename T>
	static inline T Lerp(T left, T right, float t)
	{
		return left * t + right * (1.f - t);
	}

	void Animator::OnUpdate(et::TimeStep ts)
	{
		for (auto& animation : animations)
		{
			if (!animation.animating)
				continue;

			animation.currentFrame += ((float)animation.fps * ts);
			if ((uint32_t)animation.currentFrame >= animation.keyFrames.back().frame)
				animation.animating = false;

			auto& [frame0, frame1] = animation.find((uint32_t)animation.currentFrame);
			float one = (float)(frame1.frame - frame0.frame);
			if (one == 0.f)
				animation.currentFrameOffset = frame1.position;
			else	
			{
				float t = (float)(animation.currentFrame - frame0.frame) / one;
				animation.currentFrameOffset = Lerp(frame1.position, frame0.position, t);
			}
		}
	}

	size_t Animator::Play(const AnimationInfo& animation)
	{
		// clear animations that are done playing
		auto copy = animations;
		for (auto& animation : copy)
			if (!animation.animating)
			{
				auto it = std::find(animations.begin(), animations.end(), animation);
				animations.erase(it);
			}

		animations.push_back(animation);
		animations.back().currentFrame = 0;
		animations.back().animating = true;
		animations.back().currentFrameOffset = {};

		return animations.size() - 1;
	}

	bool Animator::Animate(glm::vec2& position, size_t animation_id)
	{
		ET_DEBUG_ASSERT(animation_id < animations.size());
		position = animations[animation_id].currentFrameOffset;
		return animations[animation_id].animating;
	}
}