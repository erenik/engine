/// Emil Hedemalm
/// 2015-02-06
/// Any variable within e.g. a game. May be used for other purposes. These vars are usually saved on a per game-save basis.

#include "GameVariable.h"
#include <fstream>

GameVariable::~GameVariable()
{
}


void GameVariable::PrintType() 
{
	switch(type){
		case INTEGER: std::cout<<"Integer"; break;
		case FLOAT: std::cout<<"Float"; break;
		case STRING: std::cout<<"String"; break;
		case VECTOR: std::cout<<"Vector"; break;
		case CUSTOM: std::cout<<"Custom"; break;
		default: 
			assert(false);
			std::cout<<"Bad/no type"; break;
			break;
	}
};

String GameVariable::ToString()
{
	switch(type)
	{
		case INTEGER: return String(iValue);
		case STRING: return strValue;
		case TIME: return String(timeValue.Milliseconds());
		default:
			assert(false);
			break;
	}
	return String("Bad");
}

/// Converts to int as needed.
int GameVariable::GetInt()
{
	switch(type)
	{
		case INTEGER: return iValue;
		case INT64: return int (i64Value);
	}
	return -1;
}

/// Sets value using integer input.
void GameVariable::SetInt(int newIValue)
{
	switch(type)
	{
		case INTEGER: iValue = newIValue; break;
		default:
			assert(false);
	}
	return;
}



bool GameVariable::WriteTo(std::fstream & stream)
{
	int version = 0;
	stream.write((char*)&version, sizeof(int));
	stream.write((char*)&type, sizeof(int));
	name.WriteTo(stream);
	switch(type)
	{
		case INTEGER: stream.write((char*)&iValue, sizeof(int)); break;
		case TIME: timeValue.WriteTo(stream); break;
		case STRING: strValue.WriteTo(stream); break;
		default:
			assert(false);
	}
	return true;
}

bool GameVariable::ReadFrom(std::fstream & stream)
{
	int version = 0;
	stream.read((char*)&version, sizeof(int));
	assert(version == 0);
	if (version != 0)
		return false;
	// Type?
	stream.read((char*)&type, sizeof(int));
	name.ReadFrom(stream);
	switch(type)
	{
		case INTEGER: stream.read((char*)&iValue, sizeof(int)); break;
		case TIME: timeValue.ReadFrom(stream); break;
		case STRING: strValue.ReadFrom(stream); break;
		default:
			assert(false);
	}
	return true;
}
