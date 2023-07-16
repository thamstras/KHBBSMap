/*GLenum g_DebugLevel;

const GLuint message_excludes[] = {
	131185,			// Buffer detailed info
};
const GLsizei message_excludes_count = 1;


void GLAPIENTRY gl_DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	const char* strSource, *strType, *strSev;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API_ARB: strSource = "API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB: strSource = "WINDOW_SYSTEM"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB: strSource = "SHADER_COMPILER"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY_ARB: strSource = "THIRD_PARTY"; break;
	case GL_DEBUG_SOURCE_APPLICATION_ARB: strSource = "APPLICATION"; break;
	case GL_DEBUG_SOURCE_OTHER_ARB:
	default:
		strSource = "OTHER";
		break;
	}

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR_ARB: strType = "ERROR"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB: strType = "DEPRECATED_BEHAVIOR"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB: strType = "UNDEFINED_BEHAVIOR"; break;
	case GL_DEBUG_TYPE_PORTABILITY_ARB: strType = "PORTABILITY"; break;
	case GL_DEBUG_TYPE_PERFORMANCE_ARB: strType = "PERFORMANCE"; break;
	case GL_DEBUG_TYPE_OTHER_ARB:
	default:
		strType = "OTHER";
		break;
	}

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH_ARB: strSev = "HIGH"; break;
	case GL_DEBUG_SEVERITY_MEDIUM_ARB: strSev = "MEDIUM"; break;
	case GL_DEBUG_SEVERITY_LOW_ARB: strSev = "LOW"; break;
	default:
		strSev = "OTHER";
		break;
	}

	std::cout << "[GL DEBUG][" << strSev << "][" << strSource << "][" << strType << "][" << id << "] " << message << std::endl;
}

void setupGLDebug()
{
	glDebugMessageCallbackARB(&gl_DebugCallback, 0);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
	//glEnable(GL_DEBUG_OUTPUT);
	if (message_excludes_count)
	{
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, message_excludes_count, message_excludes, GL_FALSE);
	}
}*/