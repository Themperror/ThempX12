#include "core/renderer/shader.h"
#include "core/engine.h"
#include "core/util/svars.h"


namespace Themp::D3D
{
	void Shader::Init(const std::string& name)
	{
		m_Name = name;
	}
	void Shader::AddShaderSource(std::string name, ShaderType type)
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