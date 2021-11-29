#include "renderer/shader.h"
#include "Engine.h"
#include "util/svars.h"


namespace Themp::D3D
{
	void Shader::Init(const std::string& name)
	{
		m_Name = name;
	}
	void Shader::AddShaderSource(std::wstring name, ShaderType type)
	{
		m_ShaderPairs.push_back({ name, type });
	}
	bool Shader::IsValid() const
	{
		return m_ShaderPairs.size() > 0;
	}

	const std::vector<Shader::SourcePair>& Shader::GetShaders() const
	{
		return m_ShaderPairs;
	}
}