// Emil Hedemalm
// 2013-07-10

#ifndef UI_LIST_H
#define UI_LIST_H

#include "UIElement.h"

class UIScrollBarHandle : public UIElement {
public:
    UIScrollBarHandle();
	virtual ~UIScrollBarHandle();
    void Move(float distance);
};

class UIScrollBar : public UIElement {
    friend class UIList;
public:
    UIScrollBar();
	virtual ~UIScrollBar();
    void CreateHandle();
    void Update(float newSize);
    /// Move the scrollbar, capping it depending on it's size (will never exceed the 0.0 - 1.0 limits)
    void Move(float distance);
    /// Returns the current relative start of the contents, 0.0 being top and close to (pages) at the maximum, varying with the content length.
    float GetStart();
    /// Returns the current relative start of the contents
    float GetStop();
	/// Returns size of the scroll bar handle, equivalent to how big a page is relative to the total contents.
	float PageSize();
    void PrintDebug();
private:
    UIScrollBarHandle * handle;
    float previousSize;
};

/** The UIList class acts in such a way that it formats it's internals content after demand, by repositioning them as needed.
	As such, it uses several own functions in order to meet these demands.
*/
class UIList : public UIElement {
public:
	UIList();
	virtual ~UIList();

	/// Deletes all children and content inside.
	void Clear();
	// Adjusts hierarchy besides the regular addition
	virtual void AddChild(UIElement* child); // Sets child pointer to child UI element, NULL if non

	/// Activation functions
	virtual UIElement* Hover(int mouseX, int mouseY);
	virtual UIElement* Click(int mouseX, int mouseY);
	/// GEtttererrr
	virtual UIElement * GetElement(int mouseX, int mouseY);
	/** For mouse-scrolling. By default calls it's parent's OnScroll. Returns true if the element did anything because of the scroll.
		The delta corresponds to amount of "pages" it should scroll.
	*/
	virtual bool OnScroll(float delta);

    /// Scroll, not capped.
	virtual bool Scroll(float absoluteDistanceInPages);

    /// Adjusts positions and sizes acording to any attached scroll-bars or other system UI elements.
	void FormatElements();

	/// Rendering
	virtual void Render(GraphicsState& graphics);

	/// Called to ensure visibility of target element.
	virtual void EnsureVisibility(UIElement * element);

protected:
	/// Called whenever an element is deleted externally. Sub-class in order to properly deal with references.
//	virtual void OnElementDeleted(UIElement * element);


private:

    virtual void RenderChildren(GraphicsState & graphics);

	/// Total contents size
	float contentsSize;

    /// If the element uses scrollbars for internal content handling.
    UIScrollBar * scrollBarX, * scrollBarY;
};


/** The UIColumnList class acts in such a way that it formats it's internals content after demand, by repositioning them as needed.
	Works similar to UIList but side-ways instead ^w^
*/
class UIColumnList : public UIElement {
public:
	UIColumnList(String name = "");
	virtual ~UIColumnList();
	/// Deletes all children and content inside.
	void Clear();
	// Adjusts hierarchy besides the regular addition
	void AddChild(UIElement* child); // Sets child pointer to child UI element, NULL if non
};


#endif
