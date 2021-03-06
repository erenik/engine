/// Emil Hedemalm
/// 2014-02-17
/// A UI element that can stream/play video!

#include "UIVideo.h"
#include "UITypes.h"
#include "Multimedia/MultimediaManager.h"
#include "Graphics/Shader.h"
#include "GraphicsState.h"
#include "Texture.h"

#include "Graphics/GraphicsManager.h"

UIVideo::UIVideo(String name, String initialVideoUrl)
: UIElement(), streamUrl(initialVideoUrl)
{
	type = UIType::VIDEO;
	stream = NULL;
	streamUrl.Remove("\"", true);
	this->activationMessage = "TogglePause(this)";
	interaction.hoverable = true;
	interaction.navigatable = true;
	this->interaction.activateable = true;
	visuals.highlightOnHover = false;
	visuals.highlightOnActive = false;
}

UIVideo::~UIVideo()
{
	if (stream){
		/// Multimedia manager should deallocate the stream.
		stream->Pause();
		/// Closes target file and stream.
		stream->Close();
	}
}


/// Called when this UI is made active (again).
void UIVideo::OnEnterScope(){
	if (wasPlayingWhenExitScope && stream)
		stream->Play();
	UIElement::OnEnterScope();
}

/// Called once this element is no longer visible for any reason. E.g. switching game states to display another UI.
void UIVideo::OnExitScope(bool forced){
	if (stream && stream->IsPlaying()){
		stream->Pause();
		wasPlayingWhenExitScope = true;
	}
	UIElement::OnExitScope(forced);
}


/// Tries to create the target stream.
bool UIVideo::CreateStream()
{
	stream = MultimediaMan.Play(streamUrl);
	if (!stream)
		return false;
	
	// If successful, create our texture too?
	videoFrameTexture = stream->GetFrameTexture();
	return true;
}
/// Toggles pause for the video-stream.
void UIVideo::TogglePause()
{
	if (!stream)
		return;
	if (stream->IsPlaying())
		stream->Pause();
	else if (stream->IsPaused())
		stream->Play();
}

/// Subclassing in order to render the video!
void UIVideo::RenderSelf(GraphicsState & graphicsState)
{
	/// Old code
	UIElement::RenderSelf(graphicsState);

	// Don't go further if we don't have any stream to render.
	if (!stream)
		return;

	/// Hide this UI once the video is done unless we've got repeat set.
	if (stream->HasEnded()){
		interaction.visible = false;
		return;
	}

	/// Automatically start the stream when we render this UI.
	if (stream->IsReady() && playWhenReady){
		stream->Play();
	}
// return;

	/// Set mip-map filtering to closest
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Enable textures if it wasn't already
	glEnable(GL_TEXTURE_2D);
	/// Set fill mode!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	// Make it red.
//	videoFrameTexture->MakeRed();
	/// Rebuffer the texture as needed.
	if (videoFrameTexture->queueRebufferization)
		videoFrameTexture->Bufferize();

	/// Save old shader!
	Shader * pastShader = ActiveShader();

	ShadeMan.SetActiveShader(&graphicsState, nullptr);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glLoadMatrixf(graphicsState.projectionMatrixF.getPointer());
	Matrix4f modelView = graphicsState.viewMatrixF * graphicsState.modelMatrixF;
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(modelView.getPointer());
	glColor4f(1,1,1,1);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/// Bind it again after switching program.
	glBindTexture(GL_TEXTURE_2D, videoFrameTexture->glid);

	/// Disable depth, test, since I'm lazy.
	glDisable(GL_DEPTH_TEST);

	float videoWidth = videoFrameTexture->Width();
	float videoHeight = videoFrameTexture->Height();
	
	float aspectRatio = videoWidth / videoHeight;
	float videoLeft, videoRight, videoTop, videoBottom;
	
	float uiElementAspectRatio = ((float)layout.sizeX) / layout.sizeY;
	
	float centerX = (layout.right + layout.left) / 2.0f;
	float centerY = (layout.top + layout.bottom) / 2.0f;
	float halfSizeX = layout.sizeX / 2.0f;
	float halfSizeY = layout.sizeY / 2.0f;

	float ratioDiff = aspectRatio / uiElementAspectRatio;
	if (ratioDiff > 1.0f)
	{
		halfSizeY /= ratioDiff;		
	}
	else if (ratioDiff < 1.0f) {
		halfSizeX *= ratioDiff;
	}

	
	/// Render a quad.
	float x1 = centerX - halfSizeX, 
		x2 = centerX + halfSizeX, 
		y1 = centerY - halfSizeY, 
		y2 = centerY + halfSizeY;
	
	float texCoordX1 = 0, texCoordX2 = 1,
		texCoordY1 = 1, texCoordY2 = 0;

	glBegin(GL_QUADS);
		glTexCoord2f(texCoordX1, texCoordY1);
		glVertex3f(x1, y1, 0);
		glTexCoord2f(texCoordX2, texCoordY1);
		glVertex3f(x2, y1, 0);
		glTexCoord2f(texCoordX2, texCoordY2);
		glVertex3f(x2, y2, 0);
		glTexCoord2f(texCoordX1, texCoordY2);
		glVertex3f(x1, y2, 0);
	glEnd();
	
	ShadeMan.SetActiveShader(&graphicsState, pastShader);
}
