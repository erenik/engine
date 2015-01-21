/// Emil Hedemalm
/// 2014-07-16
/// Collision-detection for a simple Pong game

#ifndef PONG_CD_H
#define PONG_CD_H

#include "Physics/CollisionDetector.h"

class PongCD : public CollisionDetector 
{
public:
		/// Brute-force method. Does not rely on other structures that require further updates. All entities are present in the list.
	virtual int DetectCollisions(List<EntityPair> & pairs, List<Collision> & collisions);

	/// Brute-force method. Does not rely on other structures that require further updates. All entities are present in the list.
	virtual int DetectCollisions(List<Entity*> & entities, List<Collision> & collisions);

};

#endif

