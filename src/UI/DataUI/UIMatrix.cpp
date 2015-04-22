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
#include "Matrix/Matrix.h"

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

/*	if (rebuildMatrix)
	{
		CreateMatrix();
		rebuildMatrix = false;
	}
	*/
	/// TODO: Add re-formatting code in-case the size changes? Or place that in an over-loaded resize-function instead?
	UIElement::RenderSelf(graphicsState);
}


void UIMatrix::CreateChildren()
{
	/*
	/// Create a label
	label = new UILabel();
	label->text = name;
	label->sizeRatioY = 0.1f;
	label->alignmentY = 0.95f;
	AddChild(label);
*/
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

/// Adds x children. Subclassed in e.g. Matrix-class in order to setup contents properly.
bool UIMatrix::SetContents(List<UIElement*> children)
{
	DeleteContents();	
	matrixElements = children;	
	FormatContents();
	return true;
}

/// Re-arranges internal elements based on pre-configured or newly set options. Does not create or delete anything.
void UIMatrix::FormatContents()
{
	float labelHeightY = 0.0f;
//	if (label)
//		labelHeightY = label->sizeRatioY;
	float elementWidth = 1.0f / columns;
	float elementHeight = (1.0f - labelHeightY) / rows;

	Matrix<UIElement*> layoutMatrix; // Going downward in Y for each step.
	layoutMatrix.SetDefaultValue(NULL);
	layoutMatrix.SetSize(Vector2i(columns, rows));

	/// Create 'em.
	int formattedElements = 0;
	for (int y = 0; y < rows; ++y)
	{
		for (int x = 0; x < columns; ++x)
		{
			UIElement * element = matrixElements[formattedElements];
			layoutMatrix.Set(Vector2i(x, y), element);
			// Remove it first, if already there.
			RemoveChild(element);

			/// Give new alignments and size-ratios based on the matrix cell size.
			element->alignmentX = (x+0.5f) * elementWidth;
			element->alignmentY = 1.0f - (y + 0.5f) * elementHeight - labelHeightY;
			element->sizeRatioX = elementWidth;
			element->sizeRatioY = elementHeight;
			/// And add it!
			AddChild(element);
			// Make sure that the element is re-built next frame?
			++formattedElements;
			if (formattedElements >= matrixElements.Size())
				goto initialFormattingDone;
		}
	}
initialFormattingDone:
	/// After filling the matrix, set neighbour-elements accordingly.
	for (int i = 0; i < layoutMatrix.Elements(); ++i)
	{
		UIElement * element = layoutMatrix.Element(i);
		if (!element)
			continue;
		Vector2i matrixPos = layoutMatrix.GetLocationOf(element);
		/// Fetch right-left first.
		Vector2i leftPos = matrixPos + Vector2i(-1,0);
		if (layoutMatrix.ValidPosition(leftPos))
		{
			UIElement * left = layoutMatrix.GetItem(leftPos);
			if (left)
			{
				element->leftNeighbourName = left->name;
				left->rightNeighbourName = element->name;
			}
		}
	}

	;
}

/// Call before deleting or creating contents.
void UIMatrix::DeleteContents()
{
	/// If existing, delete matrix.
	while(matrixElements.Size())
	{
		UIElement * element = matrixElements[0];
		matrixElements.Remove(element);
		bool success = this->Delete(element);
		assert(success);
	}	
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

