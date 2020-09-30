// Emil Hedemalm
// 2020-08-02
// Clicking a button with Enter or pressing the A-button.

#pragma once

#include "../GMUI.h"

class GMProceedUI : public GMUI {
public:
	GMProceedUI();
	virtual ~GMProceedUI();

	virtual void Process(GraphicsState* graphicsState);
};

class GMCancelUI : public GMUI {
public:
	GMCancelUI();
	virtual ~GMCancelUI();
	virtual void Process(GraphicsState* graphicsState);
};
