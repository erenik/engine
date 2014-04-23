// Emil Hedemalm
// 2013-03-17

#include "Map.h"
#include "../Entity/CompactEntity.h"
#include "../Entity/Entity.h"
#include "../Entity/EntityManager.h"
#include "File/FileUtil.h"
#include "Script/Script.h"
#include "Pathfinding/Path.h"

#include <ctime>
#include <fstream>
#include <cstring>


#define BLOCK_ENTITIES		0x00000001
#define BLOCK_ATTRIBUTES	0x00000002
#define BLOCK_PATHS			0x00000003

/// New EOF.
#define BLOCK_END_OF_FILE	0x00001000	/// Relatively big number. GL reaching it.
#define MAX_BLOCK_TYPES		0x00001001	/// Adjust as needed

// Old numbers, avoid using but keep for compability.
#define OLD_BLOCK_END_OF_FILE	0x00000004 // Stupid giving it a four.. but whatever..
#define OLD_MAX_BLOCK_TYPES     0x00000010 // Also stupidly low number here.


bool MapFileHeader::Write(std::fstream& file){
    file.write((char*) &version, sizeof(int));
    file.write((char*) &dateCreated, sizeof(long));
    file.write((char*) mapName, MAX_PATH);
	return true;
}
bool MapFileHeader::Read(std::fstream& file){
    file.read((char*) &version, sizeof(int));
    int intSize = sizeof(int);
    int longSize = sizeof(long);
    file.read((char*) &dateCreated, sizeof(int));
    file.read((char*) mapName, MAX_PATH);
	return true;
}

void PrintPath(const char * path){
	int len = strlen(path);
	std::cout<<"\n";
	for (int i = 0; i < len; ++i){
		if (path[i] == '\\'){
			std::cout<<"/";
		}
		else {
			std::cout<<path[i];
		}
	}
}

/// Function to verify if the header was valid or not.
bool Map::VerifyHeader(){
	/// Check that we support the map verion
	if (header.version >= MV_FIRST && header.version <= MV_LATEST)
		return true;
	return false;
};

/// Saves map data to file.
bool Map::Save(const char * filename){
	std::fstream file;
	file.open(filename, std::ios_base::out | std::ios_base::binary);
	/// If failed to open, close and return false.
	if (!file.is_open()){
		file.close();
		std::cout<<"\nERROR: Unable to open filestream to ";
		PrintPath(filename);
		lastErrorString = "Unable to open file stream.";
		return false;
	}

	/// Set version to latest
	header.version = MV_LATEST;
	header.dateCreated = time(0);	// Get current time
	strcpy(header.mapName, name);	// Set header map name to the one we're using in the editor


	/// Verify that header is OK before we save it!
	if (!VerifyHeader()){
		std::cout<<"\nERROR: Unsupported map version. Try updating the game! ^^";
		return false;
	}
	int size = sizeof(MapFileHeader);	// Should be int(4) + long(4) + 260 chars
	header.Write(file);

	/// Make sure, // NO! this is check in the WriteEntities() function!
//	assert(cEntities.Size() == entities.Size());


	/// Block types for writing
	int blockType = -1;
	// Write entities if we have any
	if (NumEntities() > 0 && NumEntities() <= MAX_ENTITIES){
		WriteEntities(file);
	}
	else{
		std::cout<<"\nInvalid amount of entities: "<<NumEntities();
		lastErrorString = "No entities to save.";
		return false;
	}
	/// Write paths block if we got any
	if (paths.Size() > 0){
		WritePaths(file);
	}

	int end = BLOCK_END_OF_FILE;
	if (header.version < MV_PATHS)
		end = OLD_BLOCK_END_OF_FILE;

	file.write((char*)&end, sizeof(int));
	int endNull = 0;
	file.write((char*)&endNull, sizeof(int));

	file.close();

	/// Consider using other mechanisms to check that everything was written correctly...
	//////////////////////////////////////////
	// Check that the written contents are the same as what we wanted to save..
	if (false){
		file.open(filename, std::ios_base::in | std::ios_base::binary);
		Map verificationMap;
		file.read((char*) &verificationMap.header, sizeof(MapFileHeader));
		// Check stuff
		bool verificationDone = false;
		while(!verificationDone){
			int blockType;
			file.read((char*) &blockType, sizeof(int));
			int blockSize;
			file.read((char*) &blockSize, sizeof(int));
			switch(blockType){
				case BLOCK_ENTITIES:
					verificationMap.ReadEntities(file);
					break;
				case OLD_BLOCK_END_OF_FILE:
					verificationDone = true;
					std::cout<<"\nVerification done! File is as it should be.";
					break;
				default:
					std::cout<<"\nUnknown block type! You failed douche!";
					assert(false && "Unknown block type when verifying saved file!");
					break;
			}
		}
		file.close();
	}
	return true;
}

/// Loads map data from file.
bool Map::Load(const char * fromFile){
	std::cout<<"\nLoading map from file: "<<fromFile;
	std::fstream file;
	file.open(fromFile, std::ios_base::in | std::ios_base::binary);
	/// If failed to open, close and return false.
	if (!file.is_open()){
		file.close();
		std::cout<<"\nERROR: Unable to open filestream to ";
		lastErrorString = "Unable to open filestream.";
		PrintPath(fromFile);
		return false;
	}

	/// Read and verify header before reading any more
	header.Read(file);
	if (!VerifyHeader()){
		std::cout<<"\nERROR: Unsupported map version. Try updating the game! ^^";
		return false;
	}


	bool endOfFile = false;
	while (!file.eof() && !endOfFile){
		/// Read next integer to inrepret type of next data block
		int type;
		int blockSize;
		file.read((char*) &type, sizeof(int));
		if (header.version < MV_PATHS)
			file.read((char*) &blockSize, sizeof(int));
		if (type < 0 || type > MAX_BLOCK_TYPES){
            SWAPFOUR(type);
		}
		switch(type){
			case BLOCK_ENTITIES:
				ReadEntities(file);
				break;
			case BLOCK_PATHS:
				ReadPaths(file);
				break;
			case OLD_BLOCK_END_OF_FILE: {
				/// Check version here.
				if (header.version < MV_PATHS){
					endOfFile = true;
					break;
				}
				break;
			}
			case BLOCK_END_OF_FILE:
				endOfFile = true;
				break;
			default:
				std::cout<<"\nUnknown block type: "<<type;
				std::cout<<"\nAborting loading procedure because of unknown data in stream.";
				file.close();
				return false;
		}
	}

	/// Close file ^^
	file.close();
	source = fromFile;

	mapDataOK = true;
	std::cout<<"\nFile successfully read.";
	return true;
}

/// Reads active entity data block from file
bool Map::ReadEntities(std::fstream &file){
	// Read number of entities.
	int entitiesInBlock = 0;
	file.read((char*) &entitiesInBlock, sizeof(int));
	assert(entitiesInBlock < 10000);
	if (entitiesInBlock < 0 || entitiesInBlock > MAX_ENTITIES){
		std::cout<<"\nWARNING: Invalid amount of entities: "<<NumEntities();
		mapDataOK = false;
		return false;
	}
	// Read in the entity-data into cEntity after allocating it's size accordingly.
	for (int i = 0; i < entitiesInBlock; ++i){
		CompactEntity * cEntity = new CompactEntity();
		cEntity->ReadFrom(file);
		cEntities.Add(cEntity);
	}
	std::cout<<"\n"<<entitiesInBlock<<" entities found.";
	return true;
}
/// Writes entity data block to file
bool Map::WriteEntities(std::fstream &file){
	// TODO: Make sure cEntity corresponds to our regular entity list!
//	assert(cEntities.Size() == entities.Size());
	if (cEntities.Size())
	CLEAR_AND_DELETE(cEntities);
	//	cEntities.ClearAndDelete();

	int entitiesAdded = 0;
	for (int i = 0; i < MAX_ENTITIES && i < NumEntities(); ++i){
		if (!entities[i] || entities[i]->name.Length() == 0)
			continue;
		CompactEntity * cEntity = new CompactEntity();
		entities[i]->CreateCompactEntity(cEntity);
		cEntities.Add(cEntity);
		++entitiesAdded;
	}
	assert(cEntities.Size() == entities.Size());
	if (cEntities){
		// First write what block we're beginning
		int blockType = BLOCK_ENTITIES;
		file.write((char*) &blockType, sizeof(int));
//		int blockSize = sizeof(CompactEntity) * NumEntities() + sizeof(int);
//		file.write((char*) &blockSize, sizeof(int));
		// Write number of entities.
		file.write((char*) &cEntities.Size(), sizeof(int));
		// Then write each entity separately..?!
		for (int i = 0; i < cEntities.Size(); ++i){
			cEntities[i]->WriteTo(file);
			//file.write((char*) cEntity, sizeof(CompactEntity) * NumEntities());
		}
	}
	return true;
}


/// Loads event data, assuming that there exists an event list which at least includes 1 event and a valid source for each event (they will load themselves)
bool Map::LoadEvents(){
	std::cout<<"\nMap::LoadEvents map: "<<name;
	for(int i = 0; i < events.Size(); ++i){
		Script * event = events[i];
		/// Force-load for now, since it helps debugging. <<- wat
		if (true /*!event->loaded*/){
			event->Load(event->source);
		}
	}
	std::cout<<"\nMap::LoadEvents";
	return true;
}

/// Resets events so that they can be re-played
bool Map::ResetEvents(){
	for (int i = 0; i < events.Size(); ++i)
		events[i]->Reset();
	return true;
}

/// Loads embedded path-data, i.e. paths belonging solely to this map.
bool Map::ReadPaths(std::fstream &file){
	int numPaths;
	file.read((char*)&numPaths, sizeof(int));
	if (numPaths == 0){
		std::cout<<"\nNo paths to write, returning.";
		return true;
	}
	for (int i = 0; i < numPaths; ++i){
		Path * path = new Path();
		bool result;
		result = path->ReadFrom(file);
		if (!result)
			return false;
		paths.Add(path);
	}
	return true;
}
/// Saves embedded path-data, i.e. paths belonging solely to this map.
bool Map::WritePaths(std::fstream &file){
	int numPaths = paths.Size();
	if (numPaths == 0){
		std::cout<<"\nNo paths to write, returning.";
		return true;
	}
	int blockType = BLOCK_PATHS;
	file.write((char*)&blockType, sizeof(int));
	file.write((char*)&numPaths, sizeof(int));
	for (int i = 0; i < numPaths; ++i){
		paths[i]->WriteTo(file);
	}
	return true;
}
