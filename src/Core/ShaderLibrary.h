#pragma once
#include "Common.h"
#include "CShader.h"
#include <map>

struct ShaderDef
{
	std::string shaderName;
	std::string vertexShaderPath;
	std::string fragmentShaderPath;
};

class ShaderLibrary
{
public:
	ShaderLibrary();
	ShaderLibrary(std::vector<ShaderDef>& shaders);
	~ShaderLibrary();

	bool AddShader(ShaderDef& shaderDef);
	bool IsShaderAvailible(std::string& name);
	std::shared_ptr<CShader> GetShader(const std::string& name);
private:
	std::map<std::string, std::shared_ptr<CShader>> m_shaderMap;
};