#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace Math
{
	class Transform
	{
	private:
		enum class STATE
		{
			CLEAN,
			RECALC_VECTORS,
			RECALC_TRANSFORM
		};

		glm::vec3 m_position;
		glm::vec3 m_rotation;
		glm::vec3 m_scale;
		glm::mat4x4 m_transform;
		STATE m_state;

		void Recalc();
	public:
		Transform();
		Transform(glm::vec3 position, glm::vec3 rotation = glm::vec3(1.0f), glm::vec3 scale = glm::vec3(1.0f));
		Transform(glm::mat4x4 matrix);

		glm::vec3 Position();
		void SetPosition(glm::vec3 p);

		glm::vec3 Rotation();
		void SetRotation(glm::vec3 r);

		glm::vec3 Scale();
		void SetScale(glm::vec3 s);

		glm::mat4x4 Matrix();
		void SetMatrix(glm::mat4x4 m);
	};
}