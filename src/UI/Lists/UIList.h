// Emil Hedemalm
// 2013-07-10

#ifndef UI_LIST_H
#define UI_LIST_H

#include "UI/UIElement.h"

class UIScrollBar;

/** The UIList class acts in such a way that it formats it's internals content after demand, by repositioning them as needed.
	As such, it uses several own functions in order to meet these demands.
*/
class UIList : public UIElement {
public:
	UIList();
	virtual ~UIList();


	virtual int CenteredContentParentPosX() const override;
	// If parent is e.g. List, available size will vary depending on if scrollbar is present or not.
	virtual int AvailableParentSizeX() const override;

	void RescaleChildrenY(float f);
	/// Deletes all children and content inside.
	void Clear();
	void CreateScrollBarIfNeeded(); // Called internally.
	/** Adds a child/children
		If called with graphicsState non-NULL, it is from the render thread, and updates to the UI may be made.
	*/
	virtual bool AddChild(GraphicsState* graphicsState, UIElement* element) override;

	/// Activation functions
	virtual UIElement* Hover(int mouseX, int mouseY);
	virtual UIElement* Click(int mouseX, int mouseY);
	/// GEtttererrr
	virtual UIElement * GetElement(int mouseX, int mouseY);
	/** For mouse-scrolling. By default calls it's parent's OnScroll. Returns true if the element did anything because of the scroll.
		The delta corresponds to amount of "pages" it should scroll.
	*/
	virtual bool OnScroll(GraphicsState* graphicsState, float delta) override;

	/** Suggests a neighbour which could be to the right of this element. 
		Meant to be used for UI-navigation support. The reference element 
		indicates the element to which we are seeking a compatible or optimum neighbour, 
		and should be NULL for the initial call.

		If searchChildrenOnly is true, the call should not recurse to any parents. 
		This is set by special classes such as UIList and UIColumnList when they know
		a certain element should be or contain the correct neighbour element.
	*/
	virtual UIElement * GetUpNeighbour(GraphicsState* graphicsState, UIElement * referenceElement, bool & searchChildrenOnly) override;
	virtual UIElement * GetDownNeighbour(GraphicsState* graphicsState, UIElement * referenceElement, bool & searchChildrenOnly) override;
	virtual UIElement * GetLeftNeighbour(UIElement * referenceElement, bool & searchChildrenOnly) override;
	virtual UIElement * GetRightNeighbour(UIElement * referenceElement, bool & searchChildrenOnly) override;
	
	// Get last child which is not a system element
	UIElement * LastChild();

	/// Returns the current scroll position.
	float GetScrollPosition();
	/// Set current scroll position.
	void SetScrollPosition(float fValue);

    /// Scroll, not capped.
	virtual bool Scroll(GraphicsState* graphicsState, float absoluteDistanceInPages);

    /// Adjusts positions and sizes acording to any attached scroll-bars or other system UI elements.
	void FormatElements();

	/// Rendering
	virtual void Render(GraphicsState & graphicsState);

	/// Called to ensure visibility of target element.
	virtual void EnsureVisibility(GraphicsState* graphicsState, UIElement * element);

protected:
	/// Called whenever an element is deleted externally. Sub-class in order to properly deal with references.
//	virtual void OnElementDeleted(UIElement * element);

	// Default true.
	bool createScrollBarsAutomatically;

private:

    virtual void RenderChildren(GraphicsState & graphicsState);

	/// Total contents size
	float contentsSize;

    /// If the element uses scrollbars for internal content handling.
    UIScrollBar * scrollBarX, * scrollBarY;
};


#endif
