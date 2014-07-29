/// Emil Hedemalm
/// 2014-01-29
/// Structure for handling available games (mainly for networking)

#include "Game.h"
#include "String/StringUtil.h"

Game::Game(String name)
	: name(name)
{
	Nullify();
}

Game::Game(String name, String type, String host, String port, int currentPlayers, int maxPlayers)
: name(name), type(type), host(host), currentPlayers(currentPlayers), maxPlayers(maxPlayers)
{
	// Check that this works..
	port = port.ParseInt();
	assert(port != 0);
	Nullify();
}

// Resets variables (mainly called on creation)
void Game::Nullify()
{
	paused = false;
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

