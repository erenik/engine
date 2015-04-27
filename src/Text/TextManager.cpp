/// Emil Hedemalm
/// 2015-03-04
/// Manager for texts (localization). Used for constant strings only edited outside and before the application starts.

#include "TextManager.h"
#include "File/FileUtil.h"
#include "File/File.h"
#include "Globals.h"

TextManager * TextManager::textManager = NULL;

TextManager::TextManager()
{
	languageID = -1;
}
TextManager::~TextManager()
{
	localizedTextCategories.ClearAndDelete();
}

void TextManager::Allocate()
{
	assert(textManager == NULL);
	textManager = new TextManager();
}

void TextManager::Deallocate()
{
	SAFE_DELETE(textManager);
}

TextManager * TextManager::Instance()
{
	return textManager;
}

/// Attempts to load localized text data from given path. Default being in ./data/texts
bool TextManager::LoadFromDir(String dirPath /*= "./data/texts/"*/)
{
	List<String> files;
	int res = GetFilesInDirectory(dirPath, files);
	for (int i = 0; i < files.Size(); ++i)
	{
		String fullPath = dirPath  + "/" + files[i];
		LoadFromFile(fullPath);
	}
	return false;
}

bool TextManager::LoadFromFile(String filePath)
{
	List<String> lines = File::GetLines(filePath);
	if (lines.Size() == 0)
		return false;


}


void TextManager::SetLanguage(int newLanguageID)
{
	this->languageID = newLanguageID;
}

Text TextManager::GetText(int forTextID)
{
	return Text("BadText");
}

/// o.o
// int languageID;
/// Pretty much the languages and variations.
// List<LTC*> localizedTextCategories;

