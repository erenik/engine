// Emil Hedemalm
// 2013-07-11

#include "ShipManager.h"
#include <cmath>
#include <fstream>
#include "File/FileUtil.h"
#include "MathLib.h"
#include <cstring>
;

#undef abs

ShipManager * ShipManager::shipManager = NULL;

ShipManager::ShipManager(){}
ShipManager::~ShipManager(){
	for (int i = 0; i < ships.Size(); ++i)
		delete ships[i];
	ships.Clear();
}

ShipManager * ShipManager::Instance(){
	assert(shipManager);
	return shipManager;
}
bool ShipManager::IsAllocated(){
	return shipManager ? true : false;
}

void ShipManager::Allocate(){
	assert(shipManager == NULL);
	shipManager = new ShipManager();
}
void ShipManager::Deallocate(){
	assert(shipManager);
	delete shipManager;
	shipManager = NULL;
}

/// Looks for a "Battlers.list" which should then specify the battler-files to load.
bool ShipManager::LoadFromDirectory(String dir){
	std::cout<<"\nShipManager::LoadFromDirectory called.";
	List<String> filesInDir;
	GetFilesInDirectory(dir, filesInDir);
	std::cout<<"\nFiles in dir "<<dir<<" : "<<filesInDir.Size();
	if (filesInDir.Size() == 0){
		std::cout<<"\nERROR: No files in target directory!";
		return false;
	}
	if (dir.c_str()[dir.Length()-1] != '/')
		dir = dir + "/";
	for (int i = 0; i < filesInDir.Size(); ++i){
		String shipSource = dir + filesInDir[i];
		if (!shipSource.Contains(".s"))
			shipSource += ".s";
		Ship * ship = LoadShip(shipSource);
	}
	std::cout<<"\nShipTypes now "<<ships.Size();
	assert(ships.Size());
	return true;
}

Ship * ShipManager::LoadShip(String source){
	std::cout<<"\nShipManager::LoadShip from source: "<<source;
	std::fstream file;
	file.open(source.c_str(), std::ios_base::in);
	if (!file.is_open()){
		std::cout<<"\nERROR: Unable to open file stream to "<<source;
		file.close();
		return NULL;
	}
	int start  = (int) file.tellg();
	file.seekg( 0, std::ios::end );
	int fileSize = (int) file.tellg();
	/// Allcoate a bit more than file and nullify the ending-characters.
	int allocSize = fileSize + 3;
	char * data = new char [allocSize];
	memset(data, 0, allocSize);
	file.seekg( 0, std::ios::beg);
	file.read((char*) data, fileSize);
	file.close();
	String fileContents(data);
	delete[] data; data = NULL;
	int loadingType = 0;
	List<String> lines = fileContents.GetLines();
	Ship * ship = new Ship();
	for (int i = 0; i < lines.Size(); ++i){
		String & line = lines[i];
		// Try load the battler from the relative directory.
		if (line.Contains("//"))
			continue;
      //  line.PrintData();
		List<String > tokens = line.Tokenize(" \t");
		line.SetComparisonMode(String::NOT_CASE_SENSITIVE);
#define ASSERT_TWO_TOKENS {if(tokens.Size() < 2) continue;}
		if (line.Length() < 3)
			continue;
        if (line == ("end")){
            // End of ship file
            break;
        }
		ASSERT_TWO_TOKENS;
		if (line.Contains("name")){
			line.Remove("name");
			line.RemoveInitialWhitespaces();
			ship->name = line;
		}
		else if (line.Contains("model")){
			ship->modelSource = tokens[1];
		}
		else if (line.Contains("diffuse")){
			ship->diffuseSource = tokens[1];
		}
		else if (line.Contains("specular")){
			ship->specularSource = tokens[1];
		}
		else if (line.Contains("normal")){
			ship->normalSource = tokens[1];
		}
		else if (tokens[0] == "thrust"){
     //       std::cout<<"\nLine: "<<line;
            float thrust = tokens[1].ParseFloat();
            thrust = AbsoluteValue(thrust);
			ship->thrust =  thrust;
			std::cout<<"\nShip thrust is cool, yo: "<<ship->thrust;
			assert(AbsoluteValue(ship->thrust) > 0);
		}
		else if (line.Contains("reverse")){
			ship->reverse = AbsoluteValue(tokens[1].ParseFloat());
		}
		else if (line.Contains("angularThrust")){
			ship->angularThrust = AbsoluteValue(tokens[1].ParseFloat());
		}
		else if (line.Contains("boostMultiplier")){
			ship->boostMultiplier = AbsoluteValue(tokens[1].ParseFloat());
		}
		else if (line.Contains("maxBoost")){
			ship->maxBoost = AbsoluteValue(tokens[1].ParseFloat());
		}
		else if (line.Contains("boostRegen")){
			ship->boostRegeneration = AbsoluteValue(tokens[1].ParseFloat());
		}
		else if (line.Contains("velocityRetainedWhileTurning")){
			ship->velocityRetainedWhileTurning = AbsoluteValue(tokens[1].ParseFloat());
		}
		else if (line.Contains("thrusterPosition")){
			if (tokens.Size() < 4)
				continue;
			Vector3f pos;
			pos.x = tokens[1].ParseFloat();
			pos.y = tokens[2].ParseFloat();
			pos.z = tokens[3].ParseFloat();
			ship->thrusterPosition = pos;
		}
		else if (line.Contains("end")){
            // End of ship file
            break;
		}
		else if (line.Contains("end")){
            // End of ship file
            break;
		}
		else {
			std::cout<<"\nWARNING: ShipManager::LoadShip: Undefined line/token: "<<line;
			assert(false && "Undefined token in ShipManager::LoadShip!");
		}
	}
	bool ok = ship->HasValidStats();
	std::cout<<"\nEvaluating ship..."<< (ok? "OK" : "failed");
	assert(ok);
	assert(AbsoluteValue(ship->thrust) > 0);
	ship->source = source;
	// Remove ships with the same source, enabling re-loading!
	for (int i = 0; i < ships.Size(); ++i){
		if (ships[i]->source == ship->source){
		//	std::cout<<"\nWARNING: Battle with name "<<ship->source<<" already exists! Deleting previous entry.";
			ships.Remove(ships[i]);
		}
	}
	ships.Add(ship);
	return ship;
}

bool ShipManager::SaveShip(Ship * ship, String source){
	std::fstream file;
	file.open(source.c_str(), std::ios_base::out);
	if (!file.is_open()){
		std::cout<<"\nERROR: Unable to open file stream to "<<source;
		file.close();
		return NULL;
	}
#define LINE_END  file<<"\n";
	/// Write stuff.
	file<<"// Ship saved with ShipManager::SaveShip\n";
	LINE_END
	file<<"name\t\t"<<ship->name;LINE_END
	file<<"model\t\t"<<ship->modelSource;LINE_END
	file<<"diffuse\t\t"<<ship->diffuseSource;LINE_END
	file<<"specular\t\t"<<ship->specularSource;LINE_END
	file<<"normal\t\t"<<ship->normalSource;LINE_END
	file<<"thrust\t\t"<<ship->thrust;LINE_END
	file<<"reverse\t\t"<<ship->reverse;LINE_END
	file<<"velocityRetainedWhileTurning\t\t"<<ship->velocityRetainedWhileTurning;LINE_END
	file<<"angularThrust\t\t"<<ship->angularThrust;LINE_END
	file<<"boostMultiplier\t\t"<<ship->boostMultiplier;LINE_END
	file<<"maxBoost\t\t"<<ship->maxBoost;LINE_END
	file<<"boostRegen\t\t"<<ship->boostRegeneration;LINE_END
//	file<<"name\t"ship->name;LINE_END
	file<<"thrusterPosition\t\t"<<ship->thrusterPosition;LINE_END
	file<<"end";LINE_END

	file.close();
}


Ship * ShipManager::GetShip(String byName){
	for (int i = 0; i < ships.Size(); ++i)
		if (ships[i]->name == byName)
			return ships[i];
//	assert(false && "No ship found with given name.");
	return NULL;
}

Ship * ShipManager::GetShipBySource(String source){
	for (int i = 0; i < ships.Size(); ++i)
		if (ships[i]->source == source)
			return ships[i];
	assert(false && "No ship found with given source.");
	return NULL;
}

List<String> ShipManager::GetShipNames(){
	List<String> shipNames;
	for (int i = 0; i < ships.Size(); ++i){
		shipNames.Add(ships[i]->name);
	}
	return shipNames;
}

Ship * ShipManager::CreateShipType(String shipTypeName){
	Ship * ship = GetShip(shipTypeName);
	if (ship){
		std::cout<<"\nShip "<<shipTypeName<<" already exists! Returning it.";
		 return ship;
	}
	ship = new Ship();
	ships.Add(ship);
	return ship;
}

/*
RuneBattler ShipManager::GetBattlerType(String byName){
	assert(battlerTypes.Size() > 0);

	std::cout<<"\nGetBattlerType: by name "<<byName<<" out of "<<battlerTypes.Size()<<" types";
	for (int i = 0; i < battlerTypes.Size(); ++i){
		if (battlerTypes[i]->name == byName){
			return *battlerTypes[i];
		}
	}
	std::cout<<"\nERROR: There is no RuneBattler with name \""<<byName<<"\"!";
	assert(false && "Undefined RuneBattler type!");
	return RuneBattler();
}
*/
