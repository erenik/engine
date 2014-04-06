/// Emil Hedemalm
/// 2014-04-06
/// Sets default values for this project, like directories.

#include "ApplicationDefaults.h"

#include "UI/UserInterface.h"
#include "Maps/MapManager.h"
#include "Graphics/Fonts/Font.h"
#include "Event/Event.h"

// Main application name.
const String applicationName = "Rune RPG";	

void SetApplicationDefaults()
{
	FilePath::workingDirectory = "/SpaceRace";
	TextFont::defaultFontSource = "font3";
	Event::rootEventDir = "data/RuneRPG/Events/";

	UserInterface::rootUIDir = "gui/";
	/// This should correspond to the directory name of the released application's final install. 
	FilePath::workingDirectory = "/RuneRPG";
	MapManager::rootMapDir = "map/";
	UIElement::defaultTextureSource = "80Gray50Alpha.png";
	/*
	*/
}