/// Emil Hedemalm
/// 2014-04-06
/// Stores the application defaults function, which is called on start-up before anything else in order to get the program to start up correctly.

#ifndef APPLICATION_DEFAULTS_H
#define APPLICATION_DEFAULTS_H

#include <String/AEString.h>
#include "Graphics/Fonts/Font.h"

// Main application name is stored in the equivalent .cpp-file of your project.
extern const String applicationName;

/// Call to set application name, root directories for various features, etc.
void SetApplicationDefaults();

#endif
