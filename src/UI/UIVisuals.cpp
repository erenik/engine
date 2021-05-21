/// Emil Hedemalm
/// 2021-05-21
/// Grouping of all textures, models, and states for visuals for UI elements.

#include "UI/UIVisuals.h"

#include "Texture.h"
#include "TextureManager.h"
#include "Graphics/GLBuffers.h"
#include "Globals.h"
#include "Graphics/ShaderManager.h"
#include "Mesh/Square.h"
#include "GraphicsState.h"

#include "UI/UILayout.h"

String UIVisuals::defaultTextureSource; //  = "80Gray50Alpha.png";
// When clicking on it.
float UIVisuals::onActiveHightlightFactor = 0.3f;
// When hovering on it.
float UIVisuals::onHoverHighlightFactor = 0.0f;
// When not hovering, not clicking it.
float UIVisuals::onIdleHighlightFactor = -0.3f;
// When toggled, additional factor
float UIVisuals::onToggledHighlightFactor = 0.3f;


UIVisuals::UIVisuals() {
	Nullify();
}
UIVisuals::~UIVisuals() {
	/// Deallocate texture and mesh if needed, as well as vbo, we should never do that here though!
	assert(vboBuffer == -1 && "vboBuffer not released in UIElement::~UIElement()!");
	/// But take care of the mesh!
	SAFE_DELETE(mesh);
}


void UIVisuals::Nullify() {
	/// GL related.
	vertexArray = -1;
	// Graphical properties
	mesh = NULL;
	texture = NULL;
	vboBuffer = -1;
	vboVertexCount = 0;

	highlightOnHover = true;
	highlightOnActive = true;
	textureSource = defaultTextureSource;

	color = Vector4f(1, 1, 1, 1);

	isBuffered = isGeometryCreated = false;
}

/// Recalculates and sets highlighting factors used when rendering the UI (dedicated shader settings)
void UIVisuals::UpdateHighlightColor(bool disabled, bool isActive, bool isHover) {
	Vector3f baseColor = color;
	// If greyed out: activatable but not currently selectable/toggleable, grey it out.
	if (disabled) {
		baseColor *= 0.5f;
	}
	/// Set color if highlighted!
	Vector3f highlightColor(1, 1, 1);
	if (isActive && highlightOnActive)
		highlightColor *= onActiveHightlightFactor;
	else if (isHover && highlightOnHover) {
		highlightColor *= onHoverHighlightFactor;
	}
	else if (highlightOnHover || highlightOnActive) {
		highlightColor *= onIdleHighlightFactor;
	}
	// Duller colors for temporarily disabled buttons.
	if (disabled)
	{
		highlightColor *= 0.75f;
	}
	Shader * shader = ActiveShader();
	if (!shader)
		return;

	//	assert(activeShader->uniformPrimaryColorVec4 != -1);
	if (shader->uniformPrimaryColorVec4 == -1)
	{
		std::cout << "\nUI shader lacking primary color?";
	}
	glUniform4f(shader->uniformPrimaryColorVec4,
		baseColor[0], baseColor[1], baseColor[2], color[3]);
	if (shader->uniformHighlightColorVec4 == -1)
	{
		std::cout << "\nUI shader lacking highlight color?";
	}
	//assert(activeShader->uniformHighlightColorVec4 != -1);
	glUniform4f(shader->uniformHighlightColorVec4,
		highlightColor[0], highlightColor[1], highlightColor[2], 1.0f);
}

/** Fetches texture, assuming the textureSource has been set already. Binds and bufferizes, so call only from graphics thread.
Returns false if no texture could be find, bind or bufferized. */
bool UIVisuals::FetchBindAndBufferizeTexture() {
	// When rendering an objectwith this program.
	glActiveTexture(GL_TEXTURE0 + 0);
	/// Grab texture?
	bool validTexture = false;
	// Set texture
	if (texture && texture->glid) {
		glBindTexture(GL_TEXTURE_2D, texture->glid);
	}
	else if (texture) {
		TexMan.BufferizeTexture(texture);
	}
	else if (textureSource.Length() > 0) {
		texture = TexMan.GetTexture(textureSource);
		if (!texture)
			texture = TexMan.GetTextureByName(textureSource);
		/// Unable to fetch target-texture, so skip it.
		if (!texture) {
			glBindTexture(GL_TEXTURE_2D, 0);
			validTexture = false;
		}
		else {
			if (texture->glid == -1)
				TexMan.BufferizeTexture(texture);
			glBindTexture(GL_TEXTURE_2D, texture->glid);
		}
	}
	// No texture at all? Flag it and avoid rendering this element.
	else {
		// Use standard black texture if so, won't matter much anyway.
		texture = TexMan.GetTexture("NULL");
		glBindTexture(GL_TEXTURE_2D, texture->glid);
		validTexture = false;
		return false;
	}
	if (texture != NULL && texture->glid == -1) {
		TexMan.BufferizeTexture(texture);
	}
	if (!texture)
		return false;
	return true;
}

/// Bufferizes the UIElement mesh into the vbo buffer
void UIVisuals::Bufferize()
{
	if (!mesh) {
		isBuffered = false;
		return;
	}

	/// Check for mesh-Entity
	// Check for errors before we proceed.
	CheckGLError("UIElement::Bufferize");

	if (GL_VERSION_MAJOR >= 3) {
		// Generate VAO and bind it straight away if we're above GLEW 3.0
		if (vertexArray == -1)
			vertexArray = GLVertexArrays::New();
		// Binding caused error in wglDeleteContext later for some reason? Worth looking into later maybe.
	//	glBindVertexArray(vertexArray);
	}

	// Check for errors before we proceed.
	GLuint err = glGetError();
	if (err != GL_NO_ERROR)
		std::cout << "\nGLError buffering UI in UIElement::Bufferize";

	// Count total vertices/texture point pairs without any further optimization.
	unsigned int vertexCount = mesh->faces.Size() * 3;
	unsigned int floatsPerVertex = 3 + 3 + 2;  // Pos + Normal + UV
	unsigned int vboVertexDataLength = vertexCount * floatsPerVertex;
	float * vboVertexData = new float[vboVertexDataLength];

	// Generate And bind The Vertex Buffer
	/// Check that the buffer isn't already generated
	if (vboBuffer == -1)
	{
		vboBuffer = GLBuffers::New();
	}
	glBindBuffer(GL_ARRAY_BUFFER, vboBuffer);

	// Load all data in one big fat array, yo Data
	unsigned int vertexDataCounted = 0;
	// Reset vertices count
	vboVertexCount = 0;
	for (int i = 0; i < mesh->faces.Size(); ++i)
	{
		MeshFace * face = &mesh->faces[i];
		// Count vertices in all triangles
		for (int j = 0; j < 3; ++j)
		{
			int currentVertex = face->vertices[j];
			// Position
			vboVertexData[vertexDataCounted + 0] = mesh->vertices[currentVertex][0];
			vboVertexData[vertexDataCounted + 1] = mesh->vertices[currentVertex][1];
			vboVertexData[vertexDataCounted + 2] = mesh->vertices[currentVertex][2];
			// Normal
			int currentNormal = face->normals[j];
			vboVertexData[vertexDataCounted + 3] = mesh->normals[currentNormal][0];
			vboVertexData[vertexDataCounted + 4] = mesh->normals[currentNormal][1];
			vboVertexData[vertexDataCounted + 5] = mesh->normals[currentNormal][2];
			// UV
			int currentUV = face->uvs[j];
			vboVertexData[vertexDataCounted + 6] = mesh->uvs[currentUV][0];
			vboVertexData[vertexDataCounted + 7] = mesh->uvs[currentUV][1];
			vertexDataCounted += 8;
			++vboVertexCount;
		}
	}
	if ((unsigned int)vertexDataCounted > vboVertexDataLength)
		std::cout << "\nERROR: vertexDataCounted exceeds vertxCount * floatsPerVertex";

	// Enter the data too, fucking moron Emil-desu, yo!
	glBufferData(GL_ARRAY_BUFFER, vertexDataCounted * sizeof(float), vboVertexData, GL_STATIC_DRAW);

	// Check for errors before we proceed.
	GLuint error = glGetError();
	if (error != GL_NO_ERROR)
		std::cout << "\nGLERROR: " << error << " in UIElement::Bufferize";
	delete[] vboVertexData;
	vboVertexData = NULL;

	// Bufferize textures straight away if needed too
	if (this->texture && texture->glid == -1)
		TexMan.BufferizeTexture(texture);

	// Mark it as buffered.
	isBuffered = true;
}

void UIVisuals::FreeBuffers(GraphicsState& graphicsState) {
	if (vboBuffer != -1) {
		GLBuffers::Free(vboBuffer);
		vboBuffer = -1;
	}
	if (vertexArray != -1) {
		GLVertexArrays::Free(vertexArray);
		vertexArray = -1;
	}
	isBuffered = false;
}

void UIVisuals::CreateGeometry(GraphicsState* graphicsState, const UILayout& layout) {
	Square * sq = new Square();
	sq->SetDimensions((float)layout.left, (float)layout.right, (float)layout.bottom, (float)layout.top, layout.zDepth);
	this->mesh = sq;
	isGeometryCreated = true;
	if (retainAspectRatioOfTexture)
		ResizeGeometry(graphicsState, layout);

}
void UIVisuals::ResizeGeometry(GraphicsState* graphicsState, const UILayout& layout) {

	int right = layout.right;
	int left = layout.left;
	int top = layout.top;
	int bottom = layout.bottom;
	if (retainAspectRatioOfTexture) {
		// By default, demand dimensions to be proportional with the image.
		Vector2i size(right - left, top - bottom);
		if (!texture)
		{
			if (!FetchBindAndBufferizeTexture())
			{
				ResizeGeometry(graphicsState, layout);
				return;
			}
		}
		Vector2f texSize = texture->size;
		Vector2i center((right + left) / 2, (bottom + top) / 2);
		Vector2f elemSize(right - left, top - bottom);
		Vector2f relativeRatios = texSize / elemSize;
		// Choose the smaller ratio?
		bool xSmaller, yBigger;
		xSmaller = yBigger = relativeRatios.x < relativeRatios.y;
		float biggestPossible = relativeRatios.x > relativeRatios.y ? relativeRatios.x : relativeRatios.y;
		Vector2f biggestPossiblePx;
		if (biggestPossible > 1.f)
			biggestPossiblePx = texSize / biggestPossible;
		else if (biggestPossible > 0.f)
			biggestPossiblePx = texSize / biggestPossible;
		// Use largest possible ratio of the available space in the element.
		Vector2f modSize = biggestPossiblePx;
		// Take into account shrinking too-large spaces.
		Vector2f halfModSize = modSize * 0.5;

		this->mesh->SetDimensions((float)center.x - halfModSize.x, (float)center.x + halfModSize.x, (float)center.y - halfModSize.y, (float)center.y + halfModSize.y, layout.zDepth);
	}
	// Default, follow the previously chosen size for the element.
	else {
		//    std::cout<<"\nResizing geometry: L"<<left<<" R"<<right<<" B"<<bottom<<" T"<<top<<" Z"<<this->zDepth;
		this->mesh->SetDimensions((float)left, (float)right, (float)bottom, (float)top, layout.zDepth);
	}

	// Mark as not buffered to refresh it properly
	isBuffered = false;
}

void UIVisuals::DeleteGeometry() {
	if (mesh == NULL)
		return;
	delete mesh;
	mesh = NULL;
}

/// Splitting up the rendering.
void UIVisuals::Render(GraphicsState & graphicsState) {

	PrintGLError("GLError before UIElement::Render?!");
	assert(isGeometryCreated);
	assert(isBuffered);
	/*
	if (!isGeometryCreated)
	{
		AdjustToParent();
		CreateGeometry(&graphicsState);
	}
	if (!isBuffered)
	{
		// Re-adjust to parent.
		AdjustToParent();
		ResizeGeometry(&graphicsState);
		Bufferize();
	}
	*/

	PrintGLError("GLError in UIElement::Render 2");

	assert(vboBuffer != -1);
	/// If not buffered, do it nau!
	if (vboBuffer == -1) {
		/// If not created geometry, do that too.
		assert(this->mesh != nullptr);
		/*
		if (!this->mesh)
			CreateGeometry(&graphicsState);
		// Adjust values.
		if (parent)
			AdjustToWindow((int)parent->left, (int)parent->right, (int)parent->bottom, (int)parent->top);
		/// If parent is null, this means this is the root-element, so use current screen size?
		else
			AdjustToWindow(0, ui->Width(), 0, ui->Height());
		/// Resize the square-object.
		ResizeGeometry(&graphicsState);
		/// Buffer it.
		Bufferize();
		/// Ensure we've got a buffer now!
		*/
	}
	assert(vboBuffer && "No valid vboBuffer in UIElement::Render!");

	bool validTexture = true;


	FetchBindAndBufferizeTexture();



	/// Set mip-map filtering to closest
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	PrintGLError("GLError Binding texture");

	// Check for valid buffer before rendering,
	// Also check for valid texture...!
	if (vboBuffer && validTexture)
	{
		Shader * shader = ActiveShader();
		if (!shader)
			return;

		// Set material?	-	Not needed for UI!?
		// Just set a light-parameter to be multiplied to the texture?
		// Nothing for now... TODO

		// Set VBO and render
		// Bind vertices
		graphicsState.BindVertexArrayBuffer(vboBuffer);
		glVertexAttribPointer(shader->attributePosition, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, 0);		// Position

		// Bind UVs
		static const GLint offsetU = 6 * sizeof(GLfloat);		// Buffer already bound once at start!
		glVertexAttribPointer(shader->attributeUV, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (void *)offsetU);		// UVs

		PrintGLError("GLError Binding Buffers");
		int vertices = vboVertexCount;



		// If moveable, translate it to it's proper position!
		// TODO: Re-fix movables.
		/*
		if (movable)
		{
			///
			if (shader->uniformModelMatrix != -1) {
				/// TRanslatem power !
				Matrix4f * model = &graphicsState.modelMatrixD;
				float transX = layout.alignmentX * parent->sizeX;
				float transY = layout.alignmentY * parent->sizeY;
				model->Translate(transX, transY, 0);
				graphicsState.modelMatrixF = graphicsState.modelMatrixD;
			}
		}
		*/

		/// Load in ze model matrix
		glUniformMatrix4fv(shader->uniformModelMatrix, 1, false, graphicsState.modelMatrixF.getPointer());

		PrintGLError("GLError glUniformMatrix in UIElement");
		// Render normally
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawArrays(GL_TRIANGLES, 0, vboVertexCount);        // Draw All Of The Triangles At Once
		PrintGLError("GLError glDrawArrays in UIElement");

		// Unbind buffer
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkGLError();

	}
}

