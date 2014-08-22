/// Emil Hedemalm
/// 2014-04-06
/// Sets default values for this project, like directories.

#include "Application/Application.h"

#include "UI/UserInterface.h"
#include "Maps/MapManager.h"
#include "Graphics/Fonts/TextFont.h"
#include "Script/Script.h"

void SetApplicationDefaults()
{
	Application::name = "Rune RPG";
	TextFont::defaultFontSource = "font3";
	Script::rootEventDir = "data/RuneRPG/Events/";

	UserInterface::rootUIDir = "gui/";
	/// This should correspond to the directory name of the released application's final install.
	FilePath::workingDirectory = "RuneRPG";
	MapManager::rootMapDir = "map/";
	UIElement::defaultTextureSource = "80Gray50Alpha.png";
	/*
	*/
}