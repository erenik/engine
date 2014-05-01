// Emil Hedemalm
// 2013-07-19

#include "Graphics/GraphicsManager.h"
#include "UI/UserInterface.h"
#include "GraphicsState.h"
#include "Texture.h"

void GraphicsManager::RenderUI(UserInterface * ui){
	if (ui == NULL)
		return;
	/// Set UI Shader program
	Shader * shader = Graphics.SetShaderProgram("UI");
	if (shader == NULL && Graphics.GL_VERSION_MAJOR > 2){
	    assert(false);
		return;
    }

	// Bufferize etc. as needed.
	if (ui->AdjustToWindow(width, height))
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


    /// Setup scissor test variables
    glEnable(GL_SCISSOR_TEST);
    GraphicsState & gs = *Graphics.graphicsState;
//	std::cout<<"\nWidth: "<<Graphics.Width()<<" Height: "<<Graphics.Height();
    gs.leftScissor = 0;
	gs.rightScissor = Graphics.Width();
	gs.bottomScissor = 0;
    gs.topScissor = Graphics.Height();

    PrintGLError("GLError in RenderUI setting shader");

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	PrintGLError("GLError glBlendFunc");

	float width = ui->Width();
	float height = ui->Height();

	/// Update projection matrix
	Matrix4f projection = Matrix4f();
	projection.LoadIdentity();
	projection.InitOrthoProjectionMatrix(0, width, 0, height, -1.0f, -100.0f);
//	projection.InitProjectionMatrix(-1000, 1000, -500, 500, -1, -100);
	projection.translate(0, 0, 1.0f);

	/// Just testing that the matrix is set correctly..
	Vector4f point = Vector4f((float)width, (float)height, 0, 1.0f);
	point = projection * point;

	point = Vector4f(width/2.0f, height/2.0f, 0, 1.0f);
	point = projection * point;

	point = Vector4f(0, 0, 0, 1.0f);
	point = projection * point;

	// Load projection matrix into shader
	shader->uniformProjectionMatrix = glGetUniformLocation(shader->shaderProgram, "projectionMatrix");
	shader->uniformViewMatrix = glGetUniformLocation(shader->shaderProgram, "viewMatrix");
	shader->uniformModelMatrix = glGetUniformLocation(shader->shaderProgram, "modelMatrix");
//	std::cout<<"\nProj: "<<shader->uniformProjectionMatrix<<" view: "<<shader->uniformViewMatrix<<" model: "<<shader->uniformModelMatrix;
	PrintGLError("GLError in RenderUI getting uniform locations");

//	std::cout<<"\nShader matrix locations: "<<shader->uniformProjectionMatrix<<" "<<shader->uniformViewMatrix<<" "<<shader->uniformModelMatrix;
	assert(shader->uniformProjectionMatrix != -1);
	assert(shader->uniformViewMatrix != -1);
	assert(shader->uniformModelMatrix != -1);


    Matrix4f view = Matrix4f();

  /*
    GLchar matrixBuf[32];
    int length;
    int uniformVariableSize;
    GLenum uniformType;
    glGetActiveUniform(shader->shaderProgram,
                       shader->uniformViewMatrix,
                       32 * sizeof(char),
                       &length,
                       &uniformVariableSize,
                       &uniformType,
                       (GLchar*)&matrixBuf);
    PrintGLError("GLError in RenderUI getting active uniform for view matrix?");
*/
    // Upload view matrix too...
    glUniformMatrix4fv(shader->uniformViewMatrix, 1, false, view.getPointer());
    PrintGLError("GLError in RenderUI uploading viewMatrix");
	// Upload projection matrix
	glUniformMatrix4fv(shader->uniformProjectionMatrix, 1, false, projection.getPointer());
    PrintGLError("GLError in RenderUI uploading projectionMatrix");

	Graphics.graphicsState->projectionMatrixF = Graphics.graphicsState->projectionMatrixD = projection;
	Graphics.graphicsState->viewMatrixF = Graphics.graphicsState->viewMatrixD.LoadIdentity();
	Graphics.graphicsState->modelMatrixF = Graphics.graphicsState->modelMatrixD.LoadIdentity();

	/// Render
	if (ui == NULL)
		return;
	try {
		ui->Render(*Graphics.graphicsState);
	} catch(...){
		std::cout<<"\nERROR: Exception trying to render ui: "<<ui->Source();
	}

	// Enable alpha-blendinggg!
	glEnable(GL_BLEND);
	glDisable(GL_SCISSOR_TEST);

	// Clear errors upon entering.
	GLuint error = glGetError();
}
