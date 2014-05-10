/// Emil Hedemalm
/// 2014-03-27
/// Computer Vision Imaging application main state/settings files.

#include "ApplicationDefaults.h"

#include "FilePath/FilePath.h"
#include "Graphics/Fonts/Font.h"
#include "UI/UIElement.h"
#include "UI/UserInterface.h"

const String applicationName = "Computer Vision Imaging";

/// Call to set application name, root directories for various features, etc.
void SetApplicationDefaults()
{
	FilePath::workingDirectory = "/bin";
	TextFont::defaultFontSource = "font3";
	UIElement::defaultTextureSource = "80Gray50Alpha.png";
	UserInterface::rootUIDir = "gui/";
}

