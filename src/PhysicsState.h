/// Emil Hedemalm
/// 2015-05-06
/// Similar to the GraphicsState, handles settings and current state of physics simulation in one public static structure for easy viewing, reglardless of thread.

#ifndef PHYSICS_STATE_H
#define PHYSICS_STATE_H

class PhysicsState 
{
public:
	PhysicsState();
	bool simulationPaused;
	float simulationSpeed; // To be moved from PhysicsManager

};
extern PhysicsState * physicsState;

#endif
