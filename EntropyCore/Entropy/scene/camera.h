#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <string>

#include "Entropy/EntropyUtils.h"

namespace et
{
	class Camera
	{
	public:
		Camera() = default;
		Camera(const glm::mat4& proj) : projection(proj) {}
		~Camera() {}

		virtual void SetViewportSize(float width, float height) = 0;

		glm::mat4 projection = glm::mat4(1.f);
	};

	class OrthographicCamera : public Camera
	{
	public:
		OrthographicCamera() = default;
		OrthographicCamera(float width, float height, float depth);

		virtual void SetViewportSize(float width, float height) override;

		void Reset(float width, float height, float depth);
		void SetNear(float znear);
		void SetFar(float zfar);
		void SetSize(float size);

		float width = 0.f, height = 0.f, zNear = -1.f, zFar = 1.f;

	};

	class PerspectiveCamera : public Camera
	{
	public:
		PerspectiveCamera() = default;
		PerspectiveCamera(float vertical_fov, float aspect_ratio, float znear, float zfar);

		virtual void SetViewportSize(float width, float height) override;

		void SetFOV(float vfov);
		void SetNear(float znear);
		void SetFar(float zfar);

		float fov = 30.f, aspect = 1.f, zNear = 0.1f, zFar = 1e3f;
	};
}