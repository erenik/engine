// Emil Hedemalm
// 2021-04-25
// For Mouse input, messages sent to graphics thread for processing in the UI.

#pragma once

#include "GraphicsMessage.h"

class GMMouse : public GraphicsMessage
{
public:
	GMMouse(int interaction, AppWindow * window, Vector2i coords);
	static GMMouse * Move(AppWindow * window, Vector2i coords);
	static GMMouse * LDown(AppWindow * window, Vector2i coords);
	static GMMouse * RDown(AppWindow * window, Vector2i coords);
	static GMMouse * LUp(AppWindow * window, Vector2i coords);
	static GMMouse * RUp(AppWindow * window, Vector2i coords);
	virtual void Process(GraphicsState* graphicsState) override;
	enum interactions
	{
		MOVE,
		LDOWN,
		RDOWN,
		LUP,
		RUP,
	};
	int interaction;
	AppWindow * window;
	Vector2i coords;
};

