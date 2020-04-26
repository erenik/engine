/// Emil Hedemalm
/// 2014-02-17
/// A UI element that can stream/play video!

#ifndef UI_VIDEO_H
#define UI_VIDEO_H

#include "UIElement.h"

class MultimediaStream;

// Element integrating MultimediaStream playback into another texture which it renders within with proper aspect ratio, etc.
class UIVideo : public UIElement {
public:
	UIVideo(String name, String initialVideoUrl);
	virtual ~UIVideo();

	
	/// Called when this UI is made active (again).
	virtual void OnEnterScope();
	/// Called once this element is no longer visible for any reason. E.g. switching game states to display another UI.
	virtual void OnExitScope(bool forced);

	/// Tries to create the target stream.
	bool CreateStream();
	/// Toggles pause for the video-stream.
	void TogglePause();

	/// Subclassing in order to render the video!
	virtual void RenderSelf(GraphicsState & graphicsState);

	/// Flag to play when the element has begun being rendered.
	bool playWhenReady;
	String streamUrl;
private:
	bool wasPlayingWhenExitScope;
	MultimediaStream * stream;
	Texture * videoFrameTexture;
};


#endif