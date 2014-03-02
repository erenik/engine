/// Emil Hedemalm
/// 2014-02-28
/// Spell class that encompasses elements, effects and effect durations. 

#include "RuneSpell.h"
#include <fstream>


RuneSpell::RuneSpell()
{
	categoryName = "Rune Magic";
}
RuneSpell::~RuneSpell()
{
	effects.ClearAndDelete();
}


void RuneSpell::SetElements(String toParse)
{
	elements.Clear();
	toParse.SetComparisonMode(String::NOT_CASE_SENSITIVE);
	if (toParse.Contains("Fire"))
		elements.Add(Element::FIRE);
	if (toParse.Contains("Water"))
		elements.Add(Element::WATER);
	if (toParse.Contains("EARTH"))
		elements.Add(Element::EARTH);
	if (toParse.Contains("AIR"))
		elements.Add(Element::AIR);
	if (toParse.Contains("WATER"))
		elements.Add(Element::WATER);
	if (toParse.Contains("CHAOS"))
		elements.Add(Element::CHAOS);
	if (toParse.Contains("BALANCE"))
		elements.Add(Element::BALANCE);
	if (toParse.Contains("LIFE"))
		elements.Add(Element::LIFE);
	if (toParse.Contains("DEATH"))
		elements.Add(Element::DEATH);
}


/// Manager
RuneSpellManager * RuneSpellManager::runeSpellManager = NULL;

RuneSpellManager::RuneSpellManager()
{}

RuneSpellManager::~RuneSpellManager()
{
	spells.ClearAndDelete();
}

RuneSpellManager * RuneSpellManager::Instance()
{
	assert(runeSpellManager);
	return runeSpellManager;
}
void RuneSpellManager::Allocate()
{
	assert(runeSpellManager == NULL);
	runeSpellManager = new RuneSpellManager();
}
void RuneSpellManager::Deallocate()
{
	assert(runeSpellManager);
	delete runeSpellManager;
	runeSpellManager = NULL;
}


/// Returns list of all spells, as battle-action objects. The object will be created as usual via the BattleActionLibrary.
List<BattleAction*> RuneSpellManager::GetSpellsAsBattleActions()
{
	List<BattleAction*> list;
	for (int i = 0; i < spells.Size(); i++)
	{
		RuneSpell * spell = spells[i];
		list.Add((BattleAction*)spell);
	}
	return list;
}

/// Creates a new spell, adding it to the list and returning it.
RuneSpell * RuneSpellManager::New()
{
	RuneSpell * spell = new RuneSpell();
	spells.Add(spell);
	return spell;
}

/// Load from a CSV file (Comma-separated values).
bool RuneSpellManager::LoadFromCSV(String file)
{
	std::fstream f;
	f.open(file.c_str(), std::ios_base::in);
	if (!f.is_open()){
		f.close();
		return false;
	}
	/// Fetch all data.
	f.seekg(0, std::ios::end);
	int fileSize = (int) f.tellg();
	char * data = new char [fileSize];
	memset(data, 0, fileSize);
	f.seekg( 0, std::ios::beg);
	f.read((char*) data, fileSize);
	f.close();

	String fileContents(data);
	delete[] data; data = NULL;
	List<String> lines = fileContents.GetLines();
	int tempLineNumber;
	for (int i = 0; i < lines.Size(); ++i){
		String & line = lines[i];
		// Try load the battler from the relative directory.
		if (line.Contains("//"))
			continue;
		/// Tokenize it, taking into consideration that commas may be present, but if so inside quotation marks.
		bool insideQuotes = false;
		bool columnNamesGotten = false;
		List<String> columnNames;
		int startIndex = 0, stopIndex = line.Length() - 1;
		for (int i = 0; i < line.Length(); ++i)
		{
			/// All words, or elements, on this line
			List<String> words;
			char c = line.CharAt(i);
			switch(c){
				case '\"':
					insideQuotes = !insideQuotes;
					break;
				case ',':
				{
					if (insideQuotes)
						continue;
					/// Create the word.
					String newWord = line.Part(startIndex, i);
					words.Add(newWord);
					++i;
					startIndex = i;
					break;	
				}
			}
			/// All words parsed,
			/// If it's the first row, assume it's the category names, so save them now.
			if (columnNames.Size() == 0){
				columnNames = words;
				continue;
			}
			// If not, now loop through the words, parsing them according to the column name.
			// First create the new spell to load the data into!
			RuneSpell * spell = New();
			for (int i = 0; i < words.Size(); ++i){
				String columnName = columnNames;
				String word = words[i];
				if (columnName == "Namn" || columnName == "Name")
				{
					spell->name = word;
				}
				else if (columnName.Contains("Target"))
				{
					spell->SetTargetFilterByString(word);
				}
				else if (columnName.Contains("Location"))
				{
	//				spell->originatingLocation = GetOriginatingLocationByString(word);
				}
				else if (columnName == "Element")
				{
					spell->SetElements(word);
				}
				else if (columnName.Contains("Effect"))
				{
					// Create a new effect!
					Effect * effect = new Effect();
					effect->SetType(word);
					spell->effects.Add(effect);
				}
				else if (columnName.Contains("Duration"))
				{
					// Get latest effect and set duration for it.
					if (spell->effects.Size() == 0){
						std::cout<<"\nNo effect to attach duration to!";					
						continue;
					}
					Effect * effect = spell->effects.Last();
					effect->duration = EffectDuration::INSTANTANEOUS; // GetDurationByString(word);
				}
			}
		}
	}
}