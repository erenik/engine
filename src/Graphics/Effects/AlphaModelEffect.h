// Emil Hedemalm
// 2013-06-15

#ifndef ALPHA_MODEL_EFFECT_H
#define ALPHA_MODEL_EFFECT_H

#include "GraphicEffect.h"

class Entity;
#define EntitySharedPtr std::shared_ptr<Entity>


// Static model with interactive alpha-effects
struct AlphaModelEffect : public GraphicEffect {
	AlphaModelEffect(String name, String model, EntitySharedPtr reference);
	void SetModel();
	/// Renders, but also updates it's various values using the given timestep since last frame.
	void Render(GraphicsState & graphics);
private:
	/// Last frame's alpha, stored to avoid manual lowering of it.
	float lastAlpha;
	/** Alpha decay per frame. Linear is decremented and relative is multiplied (percentage). 
		E.g. linear decay of 0.01 will decrease from 1 to 0 in 100 seconds,
		whilst a relativeDecay of 0.1 will decrease the alpha by 10% per second.
	*/
	float linearDecay, relativeDecay;
};

#endif