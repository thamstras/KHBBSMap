#pragma once
#include "glm/glm.hpp"

namespace Graphics
{
	class ICamera
	{
	public:
		virtual ~ICamera() {};

		virtual glm::vec3 GetForwardVector() = 0;
		virtual glm::vec3 GetUpVector() = 0;
		virtual glm::vec3 GetEye() = 0;
		virtual glm::mat4 GetViewMatrix() = 0;
		//virtual Geom::Frustum GetViewFrustum() = 0;
	};
}