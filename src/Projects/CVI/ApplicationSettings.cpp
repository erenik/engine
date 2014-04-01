/// Emil Hedemalm
/// 2014-03-27
/// Computer Vision Imaging application main state/settings files.

#include "FilePath/FilePath.h"
#include "Graphics/Fonts/Font.h"
#include "UI/UIElement.h"
#include "UI/UserInterface.h"

String applicationName;

/// Call to set application name, root directories for various features, etc.
void SetApplicationDefaults()
{
	applicationName = "Computer Vision Imaging";
	FilePath::workingDirectory = "/SpaceRace";
	TextFont::defaultFontSource = "font3";
	UIElement::defaultTextureSource = "80Gray50Alpha.png";
	UserInterface::rootUIDir = "gui/";
}

