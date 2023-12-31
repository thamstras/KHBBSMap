#pragma once
#include "glm/vec4.hpp"

class Colors
{
public:
	static constexpr glm::vec4 White = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	static constexpr glm::vec4 Grey75 = glm::vec4(0.75f, 0.75f, 0.75f, 1.0f);
	static constexpr glm::vec4 Grey50 = glm::vec4(0.50f, 0.50f, 0.50f, 1.0f);
	static constexpr glm::vec4 Grey25 = glm::vec4(0.25f, 0.25f, 0.25f, 1.0f);
	static constexpr glm::vec4 Black = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	
	static constexpr glm::vec4 Red = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	static constexpr glm::vec4 Yellow = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	static constexpr glm::vec4 Green = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	static constexpr glm::vec4 Cyan = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
	static constexpr glm::vec4 Blue = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	static constexpr glm::vec4 Magenta = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
};