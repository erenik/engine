/// Emil Hedemalm
/// 2014-01-29
/// Structure for handling available games (mainly for networking)

#include "Game.h"
#include "String/StringUtil.h"

Game::Game(String name)
	: name(name)
{
}

bool Game::LoadFrom(String s){
	List<String> tokens = s.Tokenize(";");
	this->name = tokens[0];
	this->type = tokens[1];
	this->host = tokens[2];
	this->port = tokens[3].ParseInt();
	this->maxPlayers = tokens[4].ParseInt();
	this->currentPlayers = tokens[5].ParseInt();
	return true;
}

String Game::ToString(){
	List<String> ls;
	ls += name;
	ls += type;
	ls += host;
	ls += String::ToString(port);
	ls += String::ToString(maxPlayers);
	ls += String::ToString(currentPlayers);
	String s = MergeLines(ls, ";");
	return s;
}

