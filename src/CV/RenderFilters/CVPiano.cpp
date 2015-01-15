/// Emil Hedemalm
/// 2014-11-25
/// Piano via computer vision imaging interaction.

#include "CVRenderFilters.h"
#include "Audio/AudioManager.h"
#include "File/FileUtil.h"
#include "Input/InputManager.h"

CVPiano::CVPiano()
	: CVRenderFilter(CVFilterID::PIANO)
{
	playSound = new CVFilterSetting("Play sound");
	settings.Add(playSound);
}
CVPiano::~CVPiano()
{

}

int CVPiano::Process(CVPipeline * pipe)
{

	if (keySources.Size() == 0)
	{
		// Look for directory.
		LoadKeysFromDirectory("sound/instruments/piano");
		if (keySources.Size() == 0)
			keySources.Add("sound/instrments/piano/C5.wav");
	}

	if (playSound->HasChanged())
	{
		// Play stuff.
		std::cout<<"\nPlay stuff";
		AudioMan.QueueMessage(new AMPlay(AudioType::SFX, "sound/instruments/piano/C5.wav", 1.f));
	}

	// Fetch mouse position.
	PlayKey(InputMan.mousePosition);

	return CVReturnType::RENDER;
}

List<Entity*> CVPiano::GetEntities()
{
	return List<Entity*>();
}


/// o.o
void CVPiano::PlayKey(Vector2i atPosition)
{
	// Create same map for keys and locations.
	// Should already be done.

	// Check keys.
	int numKeys = keySources.Size();
	int scaledPos = atPosition.x / 100.f;

	scaledPos = scaledPos % numKeys;
	if (scaledPos < 0)
		scaledPos = 0;
	
	AudioMan.QueueMessage(new AMPlay(AudioType::SFX, keySources[scaledPos], 1.f));

}

void CVPiano::LoadKeysFromDirectory(String dir)
{
	List<String> files;
	GetFilesInDirectory(dir, files);
	for (int i = 0; i < files.Size(); ++i)
	{
		String file = files[i];
		if (file.Contains(".wav") ||
			file.Contains(".ogg")
			)
			keySources.Add(dir + "/" + file);
	}
}

