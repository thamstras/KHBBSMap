#include "Transform.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

using namespace Math;

Transform::Transform() :
	m_position(1.0f),
	m_rotation(1.0f),
	m_scale(1.0f),
	m_transform(1.0f),
	m_state(STATE::CLEAN)
{};

Transform::Transform(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) :
	m_position(position),
	m_rotation(rotation),
	m_scale(scale),
	m_transform(),
	m_state(STATE::RECALC_TRANSFORM)
{};

Transform::Transform(glm::mat4x4 matrix) :
	m_position(),
	m_rotation(),
	m_scale(),
	m_transform(matrix),
	m_state(STATE::RECALC_VECTORS)
{};

void Transform::Recalc()
{
	
	switch (m_state)
	{
	case Math::Transform::STATE::CLEAN:
		break;
	case Math::Transform::STATE::RECALC_VECTORS:
	{
		glm::quat rot;
		glm::vec3 skew;
		glm::vec4 persp;
		glm::decompose(m_transform, m_scale, rot, m_position, skew, persp);
		m_rotation = glm::eulerAngles(rot);
		m_state = STATE::CLEAN;
	}
		break;
	case Math::Transform::STATE::RECALC_TRANSFORM:
	{
		glm::quat rot = glm::quat(m_rotation);
		m_transform = glm::translate(glm::mat4x4(1.0f), m_position);
		m_transform = glm::rotate(m_transform, glm::angle(rot), glm::axis(rot));
		m_transform = glm::scale(m_transform, m_scale);
		m_state = STATE::CLEAN;
	}
		break;
	}
}

glm::vec3 Transform::Position()
{
	Recalc();
	return m_position; 
}

glm::vec3 Transform::Rotation()
{
	Recalc();
	return m_rotation; 
}

glm::vec3 Transform::Scale() 
{
	Recalc();
	return m_scale; 
}

void Transform::SetPosition(glm::vec3 p)
{
	if (m_state == STATE::RECALC_VECTORS)
		Recalc();
	m_position = p;
	m_state = STATE::RECALC_TRANSFORM;
}

void Transform::SetRotation(glm::vec3 r)
{
	if (m_state == STATE::RECALC_VECTORS)
		Recalc();
	m_rotation = r;
	m_state = STATE::RECALC_TRANSFORM;
}

void Transform::SetScale(glm::vec3 s)
{
	if (m_state == STATE::RECALC_VECTORS)
		Recalc();
	m_scale = s;
	m_state = STATE::RECALC_TRANSFORM;
}

glm::mat4x4 Transform::Matrix()
{
	Recalc();
	return m_transform;
}

void Transform::SetMatrix(glm::mat4x4 m)
{
	// NOTE: Current state doesn't actually matter, updating the matrix invalidates EVERYTHING
	//if (m_state == STATE::RECALC_TRANSFORM)
	//	Recalc();
	m_transform = m;
	m_state = STATE::RECALC_VECTORS;
}