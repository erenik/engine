/// Emil Hedemalm
/// 2014-12-01
/// For smoothly sliding values in entities.

#include "PhysicsMessage.h"
#include "Physics/PhysicsProperty.h"

#include "PhysicsLib/EstimatorFloat.h"

PMSlideEntity::PMSlideEntity(List< std::shared_ptr<Entity> > targetEntities, int target, EstimatorFloat * estimatorFloat)
	: PhysicsMessage(PM_SLIDE_ENTITY), targetEntities(targetEntities), estimatorFloat(estimatorFloat), target(target)
{
	switch(target)
	{
		case PT_ROTATION_Y:
			break;
		default:
			assert(false&&"not implemented");
			break;
	}
}
PMSlideEntity::~PMSlideEntity()
{
}
void PMSlideEntity::Process()
{
	for (int i = 0; i < targetEntities.Size(); ++i)
	{
		EntitySharedPtr entity = targetEntities[i];
		if (!entity)
		{
			std::cout<<"\nNULL entity.";
			continue;
		}
		PhysicsProperty * physics = entity->physics;
		assert(physics);
		// Create the slider (estimator)
		if (!estimatorFloat)
		{
			// Non pre-filled? 
	//		fEstimator = new EstimatorFloat();
//			fEstimator->AddState(physics->color[3], 0);
	//		fEstimator->AddState(this->targetValue, this->timeInMs);
		}
		switch(target)
		{
			case PT_ROTATION_Y:
			{
				estimatorFloat->variableToPutResultTo = &entity->rotation[1];
				break;
			}
			default:
				assert(false);
		}
		if (estimatorFloat->inheritFirstValue)
		{
			assert(estimatorFloat->variableToPutResultTo);
			estimatorFloat->InsertState(*estimatorFloat->variableToPutResultTo, 0);
		}
		physics->estimators.Add(estimatorFloat);
	}
}

PMClearEstimators::PMClearEstimators(Entities entities)
	: PhysicsMessage(PM_CLEAR_ESTIMATORS), entities(entities)
{
}
void PMClearEstimators::Process()
{
	for (int i = 0; i < entities.Size(); ++i)
	{
		EntitySharedPtr entity = entities[i];
		entity->physics->estimators.ClearAndDelete();
	}
}

