// Emil Hedemalm
// 2013-06-28

#include "TileTypeManager.h"
#include "Tile.h"
#include <fstream>
#include <sstream>
#include <cstring>

/// Singleton initialization
TileTypeManager * TileTypeManager::tileTypeManager = NULL;

/// Construct/destruct
TileTypeManager::TileTypeManager(){
}
TileTypeManager::~TileTypeManager()
{
	tileTypes.ClearAndDelete();
}

void TileTypeManager::Allocate()
{
	assert(tileTypeManager == NULL);
	tileTypeManager = new TileTypeManager();
}
void TileTypeManager::Deallocate(){
	assert(tileTypeManager);
	delete tileTypeManager;
};

TileTypeManager * TileTypeManager::Instance(){
	assert(tileTypeManager);
	return tileTypeManager;
}

void TileTypeManager::CreateDefaultTiles(){
	float l = 0.3f, m = 0.5f, f = 0.7f, v = 0.9f;
	AddTileType(new TileType(1, "Grass", Vector3f(l,f,m)));
	AddTileType(new TileType(1, "Rock", Vector3f(f,m,l)));
	AddTileType(new TileType(1, "Water", Vector3f(0,l,m)));
	AddTileType(new TileType(1, "Sand", Vector3f(v,v,l)));
	AddTileType(new TileType(1, "Snow", Vector3f(v,v,v)));
}

/// Adds
void TileTypeManager::AddTileType(TileType * type){
	tileTypes.Add(type);
}
// By name
TileType * TileTypeManager::GetTileType(String name){
	for (int i = 0; i < tileTypes.Size(); ++i)
		if (tileTypes[i]->name == name)
			return tileTypes[i];
	return NULL;
}
TileType * TileTypeManager::GetTileTypeByIndex(int i){
	if (i < 0 || i >= tileTypes.Size()){
	//	std::cout<<"\nERROR: Asking for invalid index "<<i<<" in TileTypeManager::GetTileTypeByIndex!";
		return NULL;
	}
	return tileTypes[i];
}
// Prints 'em
void TileTypeManager::PrintTypes(){
	std::cout<<"\nTileTypes:";
	for (int i = 0; i < tileTypes.Size(); ++i){
		std::cout<<"\n"<<i<<". "<<tileTypes[i]->name;
	}
}

/// Loads tile-types from specified file o-o
bool TileTypeManager::LoadTileTypes(String fromFile){
	char * data;
	int fileSize;
	std::fstream file;
	file.open(fromFile.c_str(), std::ios_base::in);
	assert(file.is_open() && "Unable to open file in TileTypeManager::LoadTileTypes");
	if (!file.is_open())
		return false;

	/// Set pointers to NULL and save indices!
	Tile::PrepareForReload();
	// Clear existing types!
	CLEAR_AND_DELETE(tileTypes);
//	tileTypes.ClearAndDelete();

	// Get size by seeking to end of file
	int start  = (int) file.tellg();
	file.seekg( 0, std::ios::end );
	fileSize = (int) file.tellg();

	// Allocate data array to required length
	data = new char [fileSize];
	memset(data, 0, fileSize);

	// Go to beginning of file and read the data
	file.seekg( 0, std::ios::beg);
	file.read((char*) data, fileSize);
	// Close file stream
	file.close();

	assert(!file.bad());

	// Parse the data?
	std::stringstream ss(data);
#define MAX_CHARS_PER_LINE 1024
	char line[MAX_CHARS_PER_LINE];
	char buf[MAX_CHARS_PER_LINE];		// Utility buffer... for temp usage
	String str;
	enum parsingState {
		NULL_STATE,
		MID_COMMENT,	 // For /* */
	};
	int parsingState = NULL_STATE;

	/// Default values that can be set when parsing
	bool defaultWalkability = true;
	Vector4f defaultColor(0,0,0,1);
	String defaultTexture;
	String defaultRootFolder;

	TileType * tt = NULL;

#define ENSURE_NEXT_TOKEN if(tokens.Size() < 2){ assert(false && "argument token missing"); continue; };
#define NEXT_TOKEN	(tokens[1])
#define SET_DEFAULTS {tt->walkability = defaultWalkability;\
	tt->color = defaultColor;\
	tt->textureSource = defaultTexture; \
	}
#define ADD_PREVIOUS_IF_NEEDED {\
	if (tt)\
	AddTileType(tt); tt = NULL;\
	}

	int tileTypesRead = 0;
	/// Read until done or too many errors!
	while (ss.good()){
		ss.getline(line, MAX_CHARS_PER_LINE);
		str = String(line);
		List<String> tokens = str.Tokenize(" \n\r\t\v\f");
		if (tokens.Size() < 1)
			continue;

		for (int t = 0; t < tokens.Size(); ++t){
			String token = tokens[t];
			token.SetComparisonMode(String::NOT_CASE_SENSITIVE);

			// Evaluate some things first depending on the current parsing state
			if (parsingState == MID_COMMENT){
				if (token.Contains("*/")){
					parsingState = NULL_STATE;
					continue;
				}
				continue;
			}
			// Regular parses
			if (token.Contains("//")){
				// Skip the rest of the line
				// Done by default at the end of these if-elseif-clauses
			}
			else if (token.Contains("/*")){
				parsingState = MID_COMMENT;
				continue;
			}
			else if (token == "defaultWalkability"){
				ENSURE_NEXT_TOKEN
				defaultWalkability = NEXT_TOKEN.ParseBool();
			}
			else if (token == "defaultTexture"){
				ENSURE_NEXT_TOKEN
				defaultTexture = NEXT_TOKEN;
				if (defaultTexture == "NULL")
					defaultTexture = String();
			}
			else if (token == "defaultRootFolder"){
				ENSURE_NEXT_TOKEN
				defaultRootFolder = NEXT_TOKEN;
			}
			else if (token == "defaultColor"){
				switch(tokens.Size()-1){
					case 1: // Assume it's alpha and keep the other colors as usual
						defaultColor.w = NEXT_TOKEN.ParseFloat();
						break;
					case 4:
						defaultColor.w = tokens[4].ParseFloat();
					case 3: // Assume it's RGB
						defaultColor.x = tokens[1].ParseFloat();
						defaultColor.y = tokens[2].ParseFloat();
						defaultColor.z = tokens[3].ParseFloat();
						break;
					case 2: case 0:
						assert(false && "Irregular amount of tokens following \"defaultTextColor\"; 1 for alpha, 3 for RGB and 4 for RGBA.");
						break;
				}
			}
			else if (token == "tileType"){
				ADD_PREVIOUS_IF_NEEDED
				tt = new TileType();
				tt->type = tileTypesRead;
				tileTypesRead++;
				if (tokens.Size() > 1)
					tt->name = NEXT_TOKEN;
				SET_DEFAULTS
			}
			else if (token == "walkable" || token == "walkability"){
				ENSURE_NEXT_TOKEN
				tt->walkability = (NEXT_TOKEN).ParseBool();
			}
			else if (token == "texture"){
				ENSURE_NEXT_TOKEN
				String param = tokens[1];
				if (param == "NULL")
					tt->textureSource = String();
				else
					tt->textureSource = defaultRootFolder + NEXT_TOKEN;
			}
			else if (token == "Color"){
				switch(tokens.Size()-1){
					case 1: // Assume it's alpha and keep the other colors as usual
						tt->color.w = NEXT_TOKEN.ParseFloat();
						break;
					case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11: case 12:
						tt->color.w = tokens[4].ParseFloat();
					case 3: // Assume it's RGB
						tt->color.x = tokens[1].ParseFloat();
						tt->color.y = tokens[2].ParseFloat();
						tt->color.z = tokens[3].ParseFloat();
						break;
					case 2: case 0:
						assert(false && "Irregular amount of tokens following \"textColor\"; 1 for alpha, 3 for RGB and 4 for RGBA.");
						break;
				}
			}
			else {
				std::cout<<"\nUnknown token in LoadTileTypes: "<<token;
				assert(false && "Unknown token in LoadTileTypes");
			}
			// By default proceed with next row straight away
			t = tokens.Size();
		}
	}
	ADD_PREVIOUS_IF_NEEDED

	if(tileTypes.Size()){
		std::cout<<"\nTileTypeManager successfully parsed "<<tileTypes.Size()<<" tileTypes!";
		Tile::Reload();
		return true;
	}
	std::cout<<"\nTileTypeManager unable to parse any tileTypes! Loading defaults again! :<";
	CreateDefaultTiles();
	Tile::Reload();
	return false;
}

TileType * TileTypeManager::GetRandom(){
	if (tileTypes.Size() == 0)
		return NULL;
	int r = rand();
	return tileTypes[r % tileTypes.Size()];
}
// Returns index of the type, or -1 if it's not a valid tile.
int TileTypeManager::Index(TileType * type){
	for (int i = 0; i < tileTypes.Size(); ++i){
		if (tileTypes[i] == type)
			return i;
	}
	return -1;
}

/// Returns next valid index available among the tiles.
int TileTypeManager::GetNextIndex(int i){
	++i;
	if (i >= tileTypes.Size())
		i = 0;
	return i;
}
TileType * TileTypeManager::GetNext(TileType * type){
	int i = Index(type);
	i = GetNextIndex(i);
	return GetTileTypeByIndex(i);
}

/// Returns previous valid index available among the tiles.
int TileTypeManager::GetPreviousIndex(int i){
	--i;
	if (i < 0)
		i = tileTypes.Size() - 1;
	return i;
}
TileType * TileTypeManager::GetPrevious(TileType * type){
	int i = Index(type);
	i = GetPreviousIndex(i);
	return GetTileTypeByIndex(i);
}
