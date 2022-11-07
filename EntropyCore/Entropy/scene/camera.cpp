#include <etpch.h>

#include "camera.h"

namespace et
{
	OrthographicCamera::OrthographicCamera(float width, float height, float depth)
		: width(width), height(height)
	{
		Reset(width, height, depth);
	}

	void OrthographicCamera::Reset(float width, float height, float depth)
	{
		zNear = -depth / 2.f;
		zFar = depth / 2.f;
		this->width = width;
		this->height = height;
		projection = glm::ortho(-width / 2.f, width / 2.f, -height / 2.f, height / 2.f, zNear, zFar);
	}

	void OrthographicCamera::SetViewportSize(float width, float height)
	{
		float aspect = width / height;
		this->width = aspect >= 0.f ? this->width : this->height * aspect;
		this->height = aspect < 0.f ? this->height : this->width / aspect;
		projection = glm::ortho(-width / 2.f, width / 2.f, -height / 2.f, height / 2.f, zNear, zFar);
	}

	void OrthographicCamera::SetNear(float znear)
	{
		zNear = znear;
		projection = glm::ortho(-width / 2.f, width / 2.f, -height / 2.f, height / 2.f, zNear, zFar);
	}

	void OrthographicCamera::SetFar(float zfar)
	{
		zFar = zfar;
		projection = glm::ortho(-width / 2.f, width / 2.f, -height / 2.f, height / 2.f, zNear, zFar);
	}

	void OrthographicCamera::SetSize(float size)
	{
		float aspect = width / height;
		width = size;
		height = width / aspect;
		projection = glm::ortho(-width / 2.f, width / 2.f, -height / 2.f, height / 2.f, zNear, zFar);
	}


	PerspectiveCamera::PerspectiveCamera(float vertical_fov, float aspect_ratio, float znear, float zfar)
		: fov(vertical_fov), aspect(aspect_ratio), zNear(znear), zFar(zfar)
	{
		projection = glm::perspective(glm::radians(vertical_fov), aspect_ratio, znear, zfar);
	}

	void PerspectiveCamera::SetViewportSize(float width, float height)
	{
		this->aspect = width / height;
		projection = glm::perspective(glm::radians(fov), aspect, zNear, zFar);
	}

	void PerspectiveCamera::SetFOV(float vfov)
	{
		fov = vfov;
		projection = glm::perspective(glm::radians(fov), aspect, zNear, zFar);
	}

	void PerspectiveCamera::SetNear(float znear)
	{
		zNear = znear;
		projection = glm::perspective(glm::radians(fov), aspect, zNear, zFar);
	}

	void PerspectiveCamera::SetFar(float zfar)
	{
		zFar = zfar;
		projection = glm::perspective(glm::radians(fov), aspect, zNear, zFar);
	}
}