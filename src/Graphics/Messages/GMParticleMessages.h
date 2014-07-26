// Emil Hedemalm
// 2013-06-16

#include "GraphicsMessage.h"

// Got generating particles using specific data.
class GMGenerateParticles : public GraphicsMessage {
public:
	GMGenerateParticles(String particleTypeName, void * extraData);
	void Process();
private:
	String name;
	void * data;
};
