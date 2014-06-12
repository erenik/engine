/// Emil Hedemalm
/// 2014-03-27
/// Computer Vision Imaging application main state/settings files.

#include "Application/Application.h"

#include "FilePath/FilePath.h"
#include "Graphics/Fonts/Font.h"
#include "UI/UIElement.h"
#include "UI/UserInterface.h"

/// Call to set application name, root directories for various features, etc.
void SetApplicationDefaults()
{
	Application::name = "Computer Vision Imaging";
	FilePath::workingDirectory = "/bin";
	TextFont::defaultFontSource = "font3";
	UIElement::defaultTextureSource = "80Gray50Alpha.png";
	UserInterface::rootUIDir = "gui/";
}

