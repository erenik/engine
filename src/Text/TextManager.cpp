/// Emil Hedemalm
/// 2015-03-04
/// Manager for texts (localization). Used for constant strings only edited outside and before the application starts.

#include "TextManager.h"
#include "File/FileUtil.h"
#include "File/File.h"
#include "Globals.h"

#include "String/StringUtil.h"

LocalizedText::LocalizedText()
: Text()
{
	id = -1;
}


LocalizedText::LocalizedText(String contents)
: Text(contents)
{
	id = -1;
}

LocalizedText LocalizedTextCategory::GetText(String byID)
{
	for (int i = 0; i < List::currentItems; ++i)
	{
		LocalizedText & lt = arr[i];
		if (lt.id == byID)
			return lt;
	}
	return LocalizedText("No text with given ID");
}


TextManager * TextManager::textManager = NULL;

TextManager::TextManager()
{
	languageID = -1;
	language = NULL;
	subtitleLanguage = NULL;
	helpTextLanguage = NULL;
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

/// Loads text-strings from default path (./data/texts/)
void TextManager::Initialize()
{
	LoadFromDir();
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
	int idIndex = -1;
	List<LTC*> ltcs;
	LocalizedText lt;
	String id;
	for (int i = 0; i < lines.Size(); ++i)
	{
		String & line = lines[i];
		List<String> tokens = TokenizeCSV(line, ';');
		for (int j = 0; j < tokens.Size(); ++j)
		{
			String token = tokens[j];
			/// Initial row.
			if (i == 0)
			{
				if (token == "ID")
					idIndex = j;
				else 
				{
					LTC * ltc = GetLanguage(token);
					if (!ltc) // New language?
					{
						ltc = new LTC();
						ltc->language = token;
						localizedTextCategories.AddItem(ltc);
					}
					if (ltc)
					{
						ltc->index = j;
						ltcs.AddItem(ltc);
					}
				}
				continue;
			}
			if (idIndex == -1)
				continue;
			// o.o
			if (j == idIndex)
				id = token;
			else 
			{
				for (int k = 0; k < ltcs.Size(); ++k)
				{
					LTC * ltc = ltcs[k];
					if (ltc->index == j)
					{
						lt = token;
						lt.id = id;
						ltc->AddItem(lt);
					}
				}
			}
		}
	}
}


void TextManager::SetLanguage(int newLanguageID)
{
	this->languageID = newLanguageID;
}

void TextManager::SetLanguage(String byName)
{
	language = GetLanguage(byName);
}

Text TextManager::GetText(String forTextID)
{
	if (language)
		return language->GetText(forTextID);
	return Text("No language selected");
}

LTC * TextManager::GetLanguage(String byName)
{
	for (int i = 0; i < localizedTextCategories.Size(); ++i)
	{
		LTC * ltc = localizedTextCategories[i];
		if (ltc->language == byName)
			return ltc;
	}
	return NULL;
}

Text TextManager::GetSubtitle(String forTextID)
{
	if (subtitleLanguage)
		return subtitleLanguage->GetText(forTextID);
	return Text("No language selected");
}
void TextManager::SetSubtitleLanguage(String byName)
{
	subtitleLanguage = GetLanguage(byName);
}

/// Used for Help
Text TextManager::GetHelpText(String forTextID)
{
	if (helpTextLanguage)
		return helpTextLanguage->GetText(forTextID);
	return Text("No language selected");
}
void TextManager::SetHelpTextLanguage(String byName)
{
	helpTextLanguage = GetLanguage(byName);
}


/// o.o
// int languageID;
/// Pretty much the languages and variations.
// List<LTC*> localizedTextCategories;

