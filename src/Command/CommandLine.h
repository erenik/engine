/// Emil Hedemalm
/// 2014-01-14
/// Command-line arg evaluator

#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

#include "String/AEString.h"

class CommandLine {
public:
	/// Evaluates the whole command-line, feeding its arguments as messages to the Message manager.
	static void Evaluate();
	/// Arguments provided on program startup. Saved here for convenience.
	static String argString;
	static List<String> args;
};

#endif
