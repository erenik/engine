/// Emil Hedemalm
/// 2014-07-10
/// Header to automatically include GL as needed. 
/// Windows requires that Windows.h is loaded first for some reason :<

#include "OpenGL.h"

/// Keeps track of GL version for the client
int GL_VERSION_MAJOR = 0;
int GL_VERSION_MINOR = 0;


struct TextError
{
	TextError(){};
	TextError(String str) : text(str)
	{
		times = 0;
	};
	String text;
	int times;
};
List<TextError> interceptedErrors;

/// Uses switch-case to print out relevant informatoin to the console. Returns the error code if any.
int CheckGLError(String inFunction)
{
	int error = glGetError();
	String text;
	text = inFunction+": "+String(error);
	switch(error)
	{
		case GL_NO_ERROR: return 0;
		case GL_INVALID_ENUM:
			text += " GL_INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			text += " GL_INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			text += " GL_INVALID_OPERATION";
			break;
		case GL_OUT_OF_MEMORY:
			text += " GL_OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			text += " GL_INVALID_FRAMEBUFFER_OPERATION";
			break;
		case GL_TABLE_TOO_LARGE:
			text += " GL_TABLE_TOO_LARGE";
			break;
		default:
			text += " Unknown error";
			assert(false);
			break;
	}
	TextError newErr(text);
	bool wasThere = false;
	int times = 0;
	for (int i = 0; i < interceptedErrors.Size(); ++i)
	{
		TextError & err = interceptedErrors[i];
		if (err.text == newErr.text)
		{
			times = err.times++;
			wasThere = true;
		}
	}
	if (!wasThere)
		interceptedErrors.AddItem(newErr);
	int skipFactor = 10;
	if (times > 100)
	{
		// Just skip it now then?
		skipFactor = 100;
	}
	if (times % skipFactor == 0)
	{
		std::cout<<"\n"<<text;
		if (times > 10)
			std::cout<<" "<<times<<"th occurance";
	}
	return error;
}
/// Similar to CheckGLError but will assert if there is an error. Returns the error code if any.
int AssertGLError(String inFunction)
{
	int error = CheckGLError(inFunction);
	assert(error == 0);
	return error;
}

#include "Window/WindowSystem.h"

/// Linux includes and globals
#ifdef USE_X11
#include <GL/glx.h>     // connect X server with OpenGL
extern void testRender();
#endif


void OpenGL::DestroyContexts()
{
	assert(false);
}


void SetupViewProjectionGL(int width, int height)
{
	glEnable(GL_DEPTH_TEST); // enable depth buffering
	glDepthFunc(GL_LESS); // pedantic, GL_LESS is the default
	glClearDepth(1.0); // pedantic, 1.0 is the default
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 1000.0);
	glViewport(0, 0, width, height);
	glEnable(GL_CULL_FACE);
	glDisable(GL_CULL_FACE);
}

void RenderTestTriangle()
{
	static float rot = 0;
	rot += 0.1f;
	RenderTestTriangle(rot);
}
void RenderTestTriangle(float triRot)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glTranslatef(0.0, 0.0, -2.0);
	glRotatef(triRot, 0.0, 1.0, 0.0);
	glBegin(GL_TRIANGLES); // front side
	glColor3f ( 1.0, 0.0, 0.0 ); // red
	glVertex3f( 0.0, 0.5, 0.0 );
	glColor3f ( 0.0, 1.0, 0.0 ); // green
	glVertex3f( -0.5, -0.5, 0.0 );
	glColor3f ( 0.0, 0.0, 1.0 ); // blue
	glVertex3f( 0.5, -0.5, 0.0 );
	glEnd();
}

