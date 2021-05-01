/// Emil Hedemalm
/// 2014-01-14
/// Input which also provides a button which pushes a FileBrowser for easy file-selection.

#pragma once

#include "UIStringInput.h"

class UIButton;

class UIFileInput : public UIStringInput {
public:
	UIFileInput(String name, String onTrigger);
	virtual ~UIFileInput();

	/// Creates the label and input.
	void CreateChildren(GraphicsState* graphicsState) override;

	// Pushes a new file browser to relevant UI where this element resides.
	void PushFileBrowser(GraphicsState * graphicsState);

	void SetFileFilter(String filter);

	void SetValue(String value) override;

private:
	String fileFilter;

	UIButton * fileBrowserButton;

};
