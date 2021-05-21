/// Emil Hedemalm
/// 2021-05-21
/// Label dedicated to displaying one-liner texts.

#pragma once

#include "UI/UIElement.h"

class UILabel : public UIElement {
public:
	UILabel(String name = "");
	virtual ~UILabel();
};
