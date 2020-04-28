/// Emil Hedemalm
/// 2014-09-08
/// A variant of a list, displaying/filtering messages, rendering each message in a specific color (if wanted)

#include "UILog.h"
#include "UITypes.h"

UILog::UILog()
	: UIList()
{
    type = UIType::LOG;
	lineSizeRatio = 0.1f;
}

/// For simple logging of just text
void UILog::Append(CTextr text)
{
	float position = this->GetScrollPosition();

	// If filled, move all texts instead? Cap 100 or so.
	if (contentChildren.Size() >= 100)
	{
		for (int i = 0; i < contentChildren.Size() - 1; ++i)
		{
			UIElement * e = contentChildren[i], * e2 = contentChildren[i+1];
			e->SetText(e2->text);
		}
		contentChildren.Last()->SetText(text);
		goto scroll;
		return;
	}
	// Add an element to self. 
	UIElement * newOne = new UIElement();
	newOne->text = text;
	newOne->sizeRatioY = lineSizeRatio;

	this->AddChild(nullptr, newOne); // Just add, since should be on graphics thread already.

scroll:
	// Start scrolling so that it becomes visible (if not already so).
	this->Scroll(nullptr, position - 1.f);
}

void UILog::Fill(List<Text> texts)
{
	for (int i = 0; i < texts.Size(); ++i)
	{
		Append(texts[i]);
	}
}



