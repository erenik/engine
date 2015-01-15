// Emil Hedemalm
// 2013-08-09

#ifndef AL_DEBUG_H
#define AL_DEBUG_H

#include "String/AEString.h"

#include "Libs.h"

#ifdef OPENAL
	void PrintALError(int alErrorCode, bool withNewLine = true);
	void PrintALError(int alErrorCode, String errorMessage);
	void CheckALError(String errorMessage);
#endif // OPENAL

#endif
