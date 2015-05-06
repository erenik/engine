/// Emil Hedemalm
/// 2015-05-06
/// Similar to the GraphicsState, handles settings and current state of physics simulation in one public static structure for easy viewing, reglardless of thread.

#include "PhysicsState.h"

/// On the stack.
PhysicsState ps;
PhysicsState * physicsState = &ps;

PhysicsState::PhysicsState()
{
	simulationPaused = false;
}

