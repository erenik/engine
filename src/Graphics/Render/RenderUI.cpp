// Emil Hedemalm
// 2013-07-19

#include "GraphicsState.h"
#include "Graphics/GraphicsManager.h"

#include "UI/UserInterface.h"
#include "Texture.h"
#include "Window/AppWindow.h"

void GraphicsManager::RenderUI(UserInterface * ui)
{
	if (ui == NULL)
		return;
	/// Set UI Shader program
	Shader * shader = ShadeMan.SetActiveShader("UI");
	if (shader == NULL && GL_VERSION_MAJOR > 2)
	{
		std::cout<<"\nUI shader unavailable for some reason. Unable to render UI.";
		return;
    }

	AppWindow * window = graphicsState->activeWindow;
	Vector2i windowWorkingArea = window->WorkingArea();
	glViewport(0, 0, windowWorkingArea[0], windowWorkingArea[1]);

	// Bufferize etc. as needed.
	if (ui->AdjustToWindow(windowWorkingArea))
	{
		if (!ui->IsGeometryCreated())
			ui->CreateGeometry();
		ui->ResizeGeometry();
		ui->Bufferize();		
	}
	
	/// Disable stuff.
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	graphicsState->currentTexture = NULL;
	// Disable lighting
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);

	// Enable blending.
	glEnable(GL_BLEND);


    /// Setup scissor test variables
    glEnable(GL_SCISSOR_TEST);
//	std::cout<<"\nWidth: "<<Graphics.Width()<<" Height: "<<Graphics.Height();
	graphicsState->scissor = Rect(0, 0, graphicsState->windowWidth, graphicsState->windowHeight);

    PrintGLError("GLError in RenderUI setting shader");
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	PrintGLError("GLError glBlendFunc");

	float width = ui->Width();
	float height = ui->Height();

	/// Update projection matrix
	Matrix4f projection = Matrix4f();
	projection.LoadIdentity();
	projection.InitOrthoProjectionMatrix(0, width, 0, height, 1.0f, 100.0f);
//	projection.InitProjectionMatrix(-1000, 1000, -500, 500, -1, -100);
	projection.Translate(0, 0, 1.0f);

	/// Just testing that the matrix is set correctly..
	Vector4f point = Vector4f((float)width, (float)height, 0, 1.0f);
	point = projection * point;

	point = Vector4f(width/2.0f, height/2.0f, 0, 1.0f);
	point = projection * point;

	point = Vector4f(0, 0, 0, 1.0f);
	point = projection * point;

	PrintGLError("GLError in RenderUI getting uniform locations");

	// Load matrices into shader
    Matrix4f view = Matrix4f();
    // Upload view matrix too...
    glUniformMatrix4fv(shader->uniformViewMatrix, 1, false, view.getPointer());
    PrintGLError("GLError in RenderUI uploading viewMatrix");
	// Upload projection matrix
	glUniformMatrix4fv(shader->uniformProjectionMatrix, 1, false, projection.getPointer());
    PrintGLError("GLError in RenderUI uploading projectionMatrix");

	graphicsState->projectionMatrixF = graphicsState->projectionMatrixD = projection;
	graphicsState->viewMatrixF = graphicsState->viewMatrixD.LoadIdentity();
	graphicsState->modelMatrixF = graphicsState->modelMatrixD.LoadIdentity();

	/// Render
	try {
		ui->Render(*graphicsState);
	} catch(...){
		std::cout<<"\nERROR: Exception trying to render ui: "<<ui->Source();
	}

	// Enable alpha-blendinggg!
	glDisable(GL_SCISSOR_TEST);

	// Clear errors upon entering.
	GLuint error = glGetError();
}
