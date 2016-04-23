/// Emil Hedemalm
/// 2014-02-25
/// A UI dedicated to handling matrices!

#include "UI/UIElement.h"

class UIMatrix : public UIElement {
	friend class UserInterface;
public:
	enum matrixTypes {
		BINARY,
	};

	UIMatrix(String name);
	virtual ~UIMatrix();
	int MatrixType(){ return matrixType;};
	
	/// Stuff.
	virtual void SetText(CTextr newText, bool force = false);

	/// Adds x children. Subclassed in e.g. Matrix-class in order to setup contents properly.
	virtual bool SetContents(List<UIElement*> children);
	/// Re-arranges internal elements based on pre-configured or newly set options. Does not create or delete anything.
	virtual void FormatContents();
	/// Call before deleting or creating contents.
	virtual void DeleteContents();

	/// Sets new column and row sizes
	virtual void SetSize(Vector2i newSize);
	/// Sets data!
	virtual void SetData(List<bool*> boolData);

	/// Callback sent to parents once an element is toggled, in order to act upon it. Used by UIMatrix.
	virtual void OnToggled(UICheckBox * box);

private:
	/// For how to display the data. With this as true (default), all booleans will have there values inverted as to how the checkboxes will be toggled or not.
	bool dataInverted;

    /// Splitting up the rendering.
    virtual void RenderSelf(GraphicsState & graphicsState);

	/// Creates the label and matrix elements.
	void CreateChildren();
	/// Creates the matrix, deleting old matrix if existing.
	void CreateMatrix();

	/// Matrix of all elements.
	List<UIElement*> matrixElements;

	/// Rebuilds matrix.
	bool rebuildMatrix;
	/// Yup.
	int rows;
	int columns;

	/// To know what kind of data is stored.
	int matrixType;
};
