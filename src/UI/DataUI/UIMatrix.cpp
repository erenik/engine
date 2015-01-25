/// Emil Hedemalm
/// 2014-02-25
/// A UI dedicated to handling matrices!

#include "UIMatrix.h"
#include "UI/UITypes.h"
#include "UI/UIButtons.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMUI.h"
#include "Message/MessageManager.h"
#include "Message/Message.h"

UIMatrix::UIMatrix(String name)
{
	type = UIType::MATRIX;
	matrixType = BINARY;
	this->name = name;
	rows = columns = 3;
	rebuildMatrix = false;
	dataInverted = true;
}

UIMatrix::~UIMatrix()
{

}

/// Splitting up the rendering.
void UIMatrix::RenderSelf(GraphicsState & graphicsState)
{
	if (rebuildMatrix)
	{
		CreateMatrix();
		rebuildMatrix = false;
	}

	UIElement::RenderSelf(graphicsState);
}


void UIMatrix::CreateChildren()
{
	/// Create a label
	label = new UILabel();
	label->text = name;
	label->sizeRatioY = 0.1f;
	label->alignmentY = 0.95f;
	AddChild(label);

	/// Booyakacha!
	CreateMatrix();
}

/// Oy.
void UIMatrix::CreateMatrix()
{
	/// If existing, delete matrix.
	while(matrixElements.Size()){
		UIElement * element = matrixElements[0];
		matrixElements.Remove(element);
		bool success = this->Delete(element);
		assert(success);
	}

	float labelHeightY = 0.0f;
	if (label)
		labelHeightY = label->sizeRatioY;
	float elementWidth = 1.0f / columns;
	float elementHeight = (1.0f - labelHeightY) / rows;

	/// Create 'em.
	for (int y = 0; y < rows; ++y)
	{
		for (int x = 0; x < columns; ++x)
		{
			UIElement * element = NULL;
			switch(matrixType){
				case BINARY:
				{
					element = new UICheckBox();
					break;
				}
				default:
					assert(false);
					continue;
			}
			element->text = "X"+String::ToString(x)+"Y"+String::ToString(y);
			element->name = name+"Element" + element->text;
			matrixElements.Add(element);
			element->textureSource = textureSource;
			element->textColor = textColor;
			element->alignmentX = (x+0.5f) * elementWidth;
			element->alignmentY = 1.0f - (y + 0.5f) * elementHeight - labelHeightY;
			element->sizeRatioX = elementWidth;
			element->sizeRatioY = elementHeight;
			AddChild(element);
		}
	}
}

/// Stuff.
void UIMatrix::SetText(Text newText, bool force)
{
	Graphics.QueueMessage(new GMSetUIs(label->name, GMUI::TEXT, newText));
}

/// Sets new column and row sizes. Only to be called when doing initial parse or from the render-thread!
void UIMatrix::SetSize(Vector2i newSize)
{
	columns = newSize[0];
	rows = newSize[1];
	CreateMatrix();
}

/// Sets data!
void UIMatrix::SetData(List<bool*> boolData)
{
	for (int i = 0; i < boolData.Size() && i < matrixElements.Size(); ++i)
	{
		int row = i / columns;
		int column = i % columns;
		if (row > rows)
			return;
		bool * data = boolData[i];
		bool bData = *data;
		if (dataInverted)
			bData = !bData;
		UIElement * element = matrixElements[i];
		element->toggled = bData;
	}
}

	/// Callback sent to parents once an element is toggled, in order to act upon it. Used by UIMatrix.
void UIMatrix::OnToggled(UICheckBox * box)
{
	/// Check which element it is, or nvm, just post shit anyway.
	DataMessage * dm = new DataMessage();
	dm->msg = "Set"+name;
	List<bool> data;
	for (int i = 0; i < matrixElements.Size(); ++i){
		UIElement * element = matrixElements[i];
		bool bValue = element->toggled;
		if (dataInverted)
			bValue = !bValue;
		data.Add(bValue);
	}
	dm->binaryData = data;
	MesMan.QueueMessage(dm);
}

