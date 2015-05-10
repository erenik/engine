/// Emil Hedemalm
/// 2015-05-10
/// Contains utility functions for handling UI. Moving code out from MessageManager so not everything is there...

#ifndef UI_UTIL_H
#define UI_UTIL_H

#include "String/AEString.h"

/// Pushes and makes visible ui by given name or source. Queues messages to the graphics thread for creation if needed.
/// Intended to be called from main non-graphics thread.
bool PushUI(String nameOrSource);
bool PopUI(String nameOrSource);

#endif
