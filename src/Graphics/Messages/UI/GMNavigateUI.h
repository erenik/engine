// Emil Hedemalm
// 2020-08-12
// Analog stick menu navigation

#pragma once

#include "../GMUI.h"
#include "NavigateDirection.h"

class GMNavigateUI : public GMUI {
public:
	// See Direction.h
	GMNavigateUI(int direction);
	GMNavigateUI(NavigateDirection direction);
	virtual ~GMNavigateUI();

	virtual void Process(GraphicsState* graphicsState);

private:
	NavigateDirection navigateDirection;
};
