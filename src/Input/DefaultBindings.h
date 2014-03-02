
#ifndef DEFAULT_BINDINGS_H
#define DEFAULT_BINDINGS_H

#include "InputManager.h"

#ifdef _DEBUG
/// Creates bindings that are used for debugging purposes only
void CreateDefaultDebuggingBindings();
#endif

void CreateDefaultGeneralBindings();
//void CreateDefaultEditorBindings();
//void CreateDefaultMenuBindings();
void CreateDefaultGameBindings();


//#include "Bindings/Pathfinding.cpp"

#endif