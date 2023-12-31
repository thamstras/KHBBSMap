#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

/*

ATTRIB LAYOUT
   | X | Y | Z | W |
--------------------
00 |   POSITION    |
01 |     COLOR     |
02 |  UV0  |       |
03 | BONE IDX 0-3  |
04 | BONE IDX 4-7  |
05 |  WEIGHT 0-3   |
06 |  WEIGHT 4-7   |
07 |
-------------------- GLES LIMIT
08 |
09 |
10 |
11 |
12 |
13 |
14 |
15 |

GL_MAX_VERTEX_ATTRIBS is at least 16 (vec4s, so 64 values)
GL_MAX_UNIFORM_LOCATIONS is at least 1024 (values, so 256 vec4s)
GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS is at least 48 (For all shader stages)
GL_MAX_*_TEXTURE_IMAGE_UNITS is at least 16 (For 1 shader stage)

TODO: Look into Uniform Blocks/Uniform Buffer Objects and GLSL Interface Blocks

NOTE: Attrib locations are "fixed" because we need to be able to bind the VAO without
      knowing what shader will be used to draw it. Uniform locations can be looked up
	  when building the shader program and stored in a map as they're always accessed
	  after the shader has been selected.
*/

/*enum class AttributeSemantic
{
	Position,
	Color,
	UV0,
	BoneIdxLow,
	BoneIdxHigh,
	BoneWeightLow,
	BoneWeightHigh
};

enum class UniformSemantic
{
	ModelTransform,
	ViewTransform,
	ProjectionTransform,

	DiffuseTexture,

	TintColor,

	UVOffset,

	FogStartDistance,
	FogEndDistance,
	FogColor,
};

struct NewShaderDef
{
	std::string shaderName;
	std::string vertexShaderPath;
	std::string fragmentShaderPath;
	std::vector<AttributeSemantic> AvailableAttributes;
	std::vector<std::pair<UniformSemantic, std::string>> UniformMap;
};*/

class ShaderAttrib
{
public:
	static constexpr std::string Position = std::string("Position");
};

class CShader
{
public:
	unsigned int ID;
	
	// constructor generates the shader on the fly
	CShader(const char* vertexPath, const char* fragmentPath);

	// activate the shader
	void use();

	// utility uniform functions
	// ------------------------------------------------------------------------

	void setBool(const std::string &name, bool value) const;

	// ------------------------------------------------------------------------

	void setInt(const std::string &name, int value) const;

	// ------------------------------------------------------------------------

	void setFloat(const std::string &name, float value) const;

	// ------------------------------------------------------------------------

	void setVec2(const std::string &name, const glm::vec2 &value) const;
	void setVec2(const std::string &name, float x, float y) const;

	// ------------------------------------------------------------------------

	void setVec3(const std::string &name, const glm::vec3 &value) const;
	void setVec3(const std::string &name, float x, float y, float z) const;

	// ------------------------------------------------------------------------

	void setVec4(const std::string &name, const glm::vec4 &value) const;
	void setVec4(const std::string &name, float x, float y, float z, float w) const;

	// ------------------------------------------------------------------------

	void setMat2(const std::string &name, const glm::mat2 &mat) const;

	// ------------------------------------------------------------------------

	void setMat3(const std::string &name, const glm::mat3 &mat) const;

	// ------------------------------------------------------------------------

	void setMat4(const std::string &name, const glm::mat4 &mat) const;

private:
	
	// utility function for checking shader compilation/linking errors.
	// ------------------------------------------------------------------------

	void checkCompileErrors(unsigned int shader, std::string type);
};
#endif