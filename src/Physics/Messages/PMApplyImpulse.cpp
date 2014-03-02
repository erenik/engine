// Emil Hedemalm
// 2013-09-03

#include "PhysicsMessage.h"
#include "Physics/PhysicsProperty.h"

PMApplyImpulse::PMApplyImpulse(List<Entity*> targetEntities, Vector3f force, Vector3f pointInSpace, float duration /* = 1.0f*/)
: PhysicsMessage(PM_APPLY_IMPULSE), entities(targetEntities), force(force), position(pointInSpace), duration(duration)
{
}

void PMApplyImpulse::Process(){
    for (int i = 0; i < entities.Size(); ++i){
        Entity * e = entities[i];

        assert(e);
        assert(e->physics);
        if (e == NULL || e->physics == NULL)
            continue;

        /// Add force and torque?
        PhysicsProperty * physics = e->physics;
		physics->ApplyImpulse(force, position);
		/*
        f->amount = force;
        f->lifeTime = duration;
        physics->forces.Add(f);

        /// Grab point locationur?
        Force * torque = new Force();

        Vector3f torqueVector = -((position) - e->positionVector).CrossProduct(force) * 0.3f;
        torque->amount = torqueVector;
        torque->lifeTime = duration;
        physics->torques.Add(torque);
		*/
    }
}
