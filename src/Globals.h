/// Emil Hedemalm
/// 2014-07-02
/// Random stuff here.

// Global definitions, constants and enumerations.

#ifndef GLOBALS_H
#define GLOBALS_H

#include "Debugging.h"

#include <cassert>
#include <sstream>
#include <cstring>

// For file system I/O
#define MAX_PATH 260

// Game version
// - 0.1
#define GAME_ENGINE_VERSION		0.1
#define GAME_VERSION			0.1

//using namespace std;
using std::stringstream;

// Stuff from book
// #define u_long unsigned long
#define GCC_NEW new(_NORMAL_BLOCK,__FILE__, __LINE__)

// Safe deletion macro
#define SAFE_DELETE(p) { if(p) { delete (p); (p) = NULL; } }
#define SAFE_DELETE_ARR(p) { if(p) { delete[] (p); (p) = NULL; } }

// File I/O
#define PATH_MAX_LENGTH MAX_PATH
/// Name handling
// #define MAX_NAME	256

// General constant settings
// const int DEFAULT_WINDOW_WIDTH = 1024;
;

//const int DEFAULT_WINDOW_HEIGHT = 768;
const bool DEFAULT_FULL_SCREEN = false;

const int TEXT_LIMIT = 240;
const int NAME_LIMIT = 32;

const int MAX_EFFECTS = 1024;


#endif
