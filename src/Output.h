/// Emil Hedemalm
/// 2015-05-20
/// Output to console, managed..

#ifndef OUTPUT_H
#define OUTPUT_H

#include "String/AEString.h"

struct TextError
{
	TextError(){};
	TextError(String str) : text(str)
	{
		times = 0;
	};
	String text;
	int times;
};

/// o.o
void Output(String text);

#endif
