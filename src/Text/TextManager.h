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
	LocalizedText();
	LocalizedText(String contents);
	// Inherit Text-functions.
	// Also provide ID and perhaps translational help?
	int id;
	String refName; // If a default name is used.
};

#define LTC LocalizedTextCategory
class LocalizedTextCategory : public List<LocalizedText>
{
public:
	LocalizedText GetText(int byID);

	String language; // Main language? E.g. English, Español, Svenska
	String locale; // Used for variations. To be added later... maybe
	int id; // assigned upon loading. Defines this combination of language and locale.
	// texts contained within the List<Text> part.
	int index; // Index while parsing from file.
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
	/// Loads text-strings from default path (./data/texts/)
	void Initialize();

	/// Attempts to load localized text data from given path. Default being in ./data/texts
	bool LoadFromDir(String dirPath = "./data/texts/");
	bool LoadFromFile(String filePath);

	void SetLanguage(int languageID);
	void SetLanguage(String byName);
	Text GetText(int forTextID);
	LTC * GetLanguage(String byName);
	/// Used for SFX and/or Help?
	Text GetSubtitle(int forTextID);
	void SetSubtitleLanguage(String byName);
	/// Used for Help
	Text GetHelpText(int forTextID);
	void SetHelpTextLanguage(String byName);
private:
	LTC * language, * subtitleLanguage, * helpTextLanguage;
	/// Active language.
	int languageID;
	/// Pretty much the languages and variations.
	List<LTC*> localizedTextCategories;
};

#endif
