/// Emil Hedemalm
/// 2015-03-04
/// Manager for texts (localization). Used for constant strings only edited outside and before the application starts.

#ifndef TEXT_MANAGER_H
#define TEXT_MANAGER_H

#define TextMan (*TextManager::Instance())

#include "Util/String/Text.h"

class LocalizedText : public Text 
{
public:
	// Inherit Text-functions.
	// Also provide ID and perhaps translational help?
	int id;
};

#define LTC LocalizedTextCategory
class LocalizedTextCategory : public List<LocalizedText>
{
public:
	String language;
	String locale; // Used for variations
	int id; // assigned upon loading. Defines this combination of language and locale.
	// texts contained within the List<Text> part.
};

class TextManager 
{
	TextManager();
	~TextManager();
	static TextManager * textManager;
public:
	static void Allocate();
	static void Deallocate();
	static TextManager * Instance();

	/// Attempts to load localized text data from given path. Default being in ./data/texts
	bool LoadFromDir(String dirPath = "./data/texts/");
	bool LoadFromFile(String filePath);

	void SetLanguage(int languageID);
	Text GetText(int forTextID);
private:
	/// o.o
	int languageID;
	/// Pretty much the languages and variations.
	List<LTC*> localizedTextCategories;
};

#endif
