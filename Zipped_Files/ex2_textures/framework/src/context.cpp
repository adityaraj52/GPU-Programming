#include "context.hpp"
#include "gl.hpp"
#include <iostream>
#include <string>

static void glDebugOutput(GLenum _source, GLenum _type, GLuint _id, GLenum _severity, GLsizei _length, const GLchar* _message, const void* _userParam)
{
	std::string debSource, debType, debSev;

	if (_source == GL_DEBUG_SOURCE_API)
		debSource = "OpenGL";
	else if (_source == GL_DEBUG_SOURCE_WINDOW_SYSTEM)
		debSource = "Windows";
	else if (_source == GL_DEBUG_SOURCE_SHADER_COMPILER)
		debSource = "Shader Compiler";
	else if (_source == GL_DEBUG_SOURCE_THIRD_PARTY)
		debSource = "Third Party";
	else if (_source == GL_DEBUG_SOURCE_APPLICATION)
		debSource = "Application";
	else if (_source == GL_DEBUG_SOURCE_OTHER)
		debSource = "Other";

	if (_type == GL_DEBUG_TYPE_ERROR)
		debType = "error";
	else if (_type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR)
		debType = "deprecated behavior";
	else if (_type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)
		debType = "undefined behavior";
	else if (_type == GL_DEBUG_TYPE_PORTABILITY)
		debType = "portability";
	else if (_type == GL_DEBUG_TYPE_PERFORMANCE)
		debType = "performance";
	else if (_type == GL_DEBUG_TYPE_OTHER)
		debType = "message";

	if (_severity == GL_DEBUG_SEVERITY_HIGH)
		debSev = "high";
	else if (_severity == GL_DEBUG_SEVERITY_MEDIUM)
		debSev = "medium";
	else if (_severity == GL_DEBUG_SEVERITY_LOW)
		debSev = "low";
	else if (_severity == GL_DEBUG_SEVERITY_NOTIFICATION)
		debSev = "note";

	std::string logMessage = debSource + ": " + debType + "(" + debSev + ") " + std::to_string(_id) + ": " + _message;
	if (_type == GL_DEBUG_TYPE_ERROR || _type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)
		throw std::exception(logMessage.c_str());
	else if (_type == GL_DEBUG_TYPE_PERFORMANCE)
		std::cerr << "INF: " << logMessage.c_str() << '\n';
	else
		std::cerr << "WAR: " << logMessage.c_str() << '\n';
}

gpupro::OGLContext::OGLContext(DebugSeverity _dbgLevel)
{
	if(!gladLoadGL())
		throw std::exception("Cannot initialize Glad/load gl-function pointers!\n");
	std::cerr << "INF: Loaded GL-context is version " << GLVersion.major << '.' << GLVersion.minor << '\n';

	// Disable or enable the different levels
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, _dbgLevel <= DebugSeverity::NOTIFICATION ? GL_TRUE : GL_FALSE);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, _dbgLevel <= DebugSeverity::LOW ? GL_TRUE : GL_FALSE);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, _dbgLevel <= DebugSeverity::MEDIUM ? GL_TRUE : GL_FALSE);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, _dbgLevel <= DebugSeverity::HIGH ? GL_TRUE : GL_FALSE);
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(&glDebugOutput, nullptr);

	// Enable seamless cube map sampling which is always a good idea since
	// it is core (3.2).
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void gpupro::OGLContext::setState(Pipeline& _pipeline)
{
	// ***** Sampler states ***************************************************
	for(int i = 0; i < 64; ++i)
	{
		if(m_currentState.samplerState[i] != _pipeline.samplerState[i])
		{
			glBindSampler(i, _pipeline.samplerState[i] ? _pipeline.samplerState[i]->glID() : 0);
			m_currentState.samplerState[i] = _pipeline.samplerState[i];
		}
	}

	// ***** Shader program ***************************************************
	if(_pipeline.shader) glUseProgram(_pipeline.shader->glID());

	static VertexFormat dummyVertexFormat({{}});
	if(_pipeline.vertexFormat) glBindVertexArray(_pipeline.vertexFormat->glID());
	else glBindVertexArray(dummyVertexFormat.glID());
}

