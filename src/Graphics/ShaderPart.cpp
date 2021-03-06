/// Emil Hedemalm
/// 2014-09-16
/// Specifies an individual shader part (a.k.a. kernel)

#include "ShaderPart.h"

#include <fstream>
#include "File/LogFile.h"
#include "Shader.h"

/// 
ShaderPart::ShaderPart(int type, Shader * owner)
: type(type), shader(owner)
{
	shaderKernelPart = -1;
}

void ShaderPart::Delete()
{
	if (shaderKernelPart)
	{
		int deleteStatus = -1;
		glGetShaderiv(shaderKernelPart, GL_DELETE_STATUS, &deleteStatus);
		if (deleteStatus == GL_FALSE)
			glDeleteShader(shaderKernelPart);
		shaderKernelPart = NULL;
	}
}

// Loads from source.
bool ShaderPart::Load(String fromSource)
{
	source = fromSource;
	if (!source.Open())
	{
		LogGraphics("Unable to open file "+fromSource, ERROR);
		return false;
	}
	CreateShaderKernelIfNeeded();
	int size = 0, start = 0;
	String sourceCode;
	// Open file
	sourceCode = source.GetContents();

	/// Check for variables within. E.g. defines.
	List<String> lines = sourceCode.GetLines();
	for (int i = 0; i < lines.Size(); ++i)
	{
		String line = lines[i];
		if (line.StartsWith("#define MAX_LIGHTS"))
		{
			shader->maxLights = (line - "#define MAX_LIGHTS").ParseInt();
		}
	}

//		sourceCode.PrintData();
	const char * arr = sourceCode.c_str();
	// Associate the source code buffers with each handle 
//	glShaderSource(shaderKernelPart, 1, (const GLchar **) &(sourceCode.c_str()), 0);
	glShaderSource(shaderKernelPart, 1, (const GLchar **) &arr, 0);
	return true;
}

void ShaderPart::CreateShaderKernelIfNeeded()
{
	if (shaderKernelPart == -1)
	{
		switch(type)
		{
			case ShaderType::VERTEX_SHADER:
				shaderKernelPart = glCreateShader(GL_VERTEX_SHADER);
				break;
			case ShaderType::FRAGMENT_SHADER:
				shaderKernelPart = glCreateShader(GL_FRAGMENT_SHADER);
				break;
		}
	}
}
// Copmpiles it, returning the result.
bool ShaderPart::Compile()
{
	CreateShaderKernelIfNeeded();
	
	/* Compile our shader objects */
	glCompileShader(shaderKernelPart);
	
	// Check if compilation failed.
	int status = NULL;
	glGetShaderiv(shaderKernelPart, GL_COMPILE_STATUS, &status);
	// If compilation failed, extract info log o-o
	int resultLength = 0;
	if (status == GL_FALSE){
		glGetShaderInfoLog(shaderKernelPart, LOG_MAX, &resultLength, shaderLog);
		if (resultLength > 0){
			// Just return if compilation fails for now. Enter a warning here or debug breakpoint if needed ^^
			String cError = "\nCompilation Error: ";
			cError += source.Path();
			cError += ": Failed compiling vertex shader: \n";
			cError += shaderLog;

			std::cout<<cError;
			glGetError();	// Release any eventual errors
			
			std::fstream dump;
			String dumpFileName = "ShaderCompilationError_";
			switch(type)
			{
				case ShaderType::FRAGMENT_SHADER:
					dumpFileName += "FragmentShader_";
					break;
				case ShaderType::VERTEX_SHADER:
					dumpFileName += "VertexShader_";
					break;
			}
			dumpFileName += name;
			dumpFileName += ".log";
			dumpFileName.Replace("/", "_");
			dump.open(dumpFileName.c_str(), std::ios_base::out);
			if (dump.is_open()){
				dump << cError;
			}
			dump.close();
			return false;
		}
	}
	return true;
}

