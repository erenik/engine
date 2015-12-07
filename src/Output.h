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

/// Prints every 10th and later 100th time (and initial one). Returns false if it doesn't print.
bool Output(String text, List<TextError> * previousErrors = 0);

#endif
