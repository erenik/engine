// Emil Hedemalm
// 2013-07-10

#include "UITypes.h"
#include "UIList.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMUI.h"
#include "GraphicsState.h"
#include "MathLib/Rect.h"

UIScrollBarHandle::UIScrollBarHandle()
: UIElement()
{
    type = UIType::SCROLL_HANDLE;
    name = "Scroll bar handle";
    selectable = true;
    hoverable = true;
    activateable = true;
    isSysElement = true;
    activationMessage = "BeginScroll(this)";
}

UIScrollBarHandle::~UIScrollBarHandle()
{
	std::cout<<"\nUIScrollBarHandle destructor.";
}

void UIScrollBarHandle::Move(float distance){
    alignmentY += distance;
    if (alignmentY > 1.0f - sizeRatioY * 0.5f)
        alignmentY = 1.0f - sizeRatioY * 0.5f;
    else if (alignmentY < 0.0f + sizeRatioY * 0.5f)
        alignmentY = 0.0f + sizeRatioY * 0.5f;
}

UIScrollBar::UIScrollBar()
: UIElement()
{
    type = UIType::SCROLL_BAR;
    name = "Scroll bar";
    handle = NULL;

    selectable = true;
    hoverable = true;
    activateable = true;
    isSysElement = true;

    activationMessage = "BeginScroll(this)";
}

UIScrollBar::~UIScrollBar()
{
	std::cout<<"\nUIScrollBar destructor.";
}

void UIScrollBar::CreateHandle(){
    assert(handle == NULL);
    handle = new UIScrollBarHandle();
    handle->textureSource = "img/80Gray50Alpha.png";
 //   handle->text = "Joooooo";
    AddChild(handle);
    previousSize = 1.0f;
}

void UIScrollBar::Update(float newSize){
    std::cout<<"\nUIScrollBar::Update New size: "<<newSize;
    float newPreviousSize = newSize;
    /// Adjust the handle's size and position.
    if (newSize <= 1.0f){
        handle->sizeRatioY = 1.0f;
        handle->alignmentY = 0.5f;
        visible = false;
    }
    else {
        float top = handle->alignmentY + handle->sizeRatioY * 0.5f;
        handle->sizeRatioY = 1.0f / newSize;
        std::cout<<"\nHandle sizeRatioY: "<<handle->sizeRatioY;
        handle->alignmentY = top - handle->sizeRatioY * 0.5f;
        std::cout<<"\nHandle alignmentY: "<<handle->alignmentY;
        visible = true;
    }
    previousSize = newPreviousSize;
}

void UIScrollBar::Move(float distance){
    handle->Move(distance);
}

/// Returns the current relative start of the contents
float UIScrollBar::GetStart(){
    float pageSize = handle->sizeRatioY;
    float handleStart = handle->alignmentY + handle->sizeRatioY * 0.5f;
    /// Divide by the size ratio to get how many pages down we are!
    float start = (1.0f - handleStart) / handle->sizeRatioY;
/*    std::cout<<"\nHandle alignmentY: "<<handle->alignmentY<<" sizeRatioY: "<<handle->sizeRatioY;
    std::cout<<"\nPage size: "<<pageSize;
    std::cout<<"\nHandle start: "<<handleStart;
    std::cout<<"\nStart: "<<start;
    */
    return start;
}
/// Returns the current relative start of the contents
float UIScrollBar::GetStop(){
    float contentSize = 1.0f / handle->sizeRatioY;
    float handleStop = handle->alignmentY - handle->sizeRatioY * 0.5f;
    float stop = (1.0f - handleStop) * contentSize;
    return stop;
}

/// Returns size of the scroll bar handle, equivalent to how big a page is relative to the total contents.
float UIScrollBar::PageSize(){
	if (!handle)
		return 1.0f;
	return this->handle->sizeRatioY;
}

void UIScrollBar::PrintDebug(){
    float pageSize = handle->sizeRatioY;
    float handleStart = handle->alignmentY + handle->sizeRatioY * 0.5f;
    float start = (1.0f - handleStart) * pageSize;

    std::cout<<"\nHandle alignmentY: "<<handle->alignmentY<<" sizeRatioY: "<<handle->sizeRatioY;
    std::cout<<"\nPage size: "<<pageSize;
    std::cout<<"\nHandle start: "<<handleStart;
    std::cout<<"\nStart: "<<start;
}


UIList::UIList()
: UIElement()
{
    type = UIType::LIST;
    formatX = true;
    scrollBarX = scrollBarY = NULL;
	contentsSize = 0.0f;
	hoverable = false;
}

UIList::~UIList()
{
	std::cout<<"\nUIList destructor.";
}

/// Deletes all children and content inside.
void UIList::Clear(){
    for (int i = 0; i < childList.Size(); /* Empty */){
    	UIElement * c = childList[i];
		if (c->isSysElement){
    	    ++i;
            continue;
        }
		childList.Remove(c);
		if (c->vboBuffer)
            c->FreeBuffers();
        if (c->mesh)
            c->DeleteGeometry();
		delete c;
		c = NULL;
    }
}

// Adjusts hierarchy besides the regular addition
void UIList::AddChild(UIElement* child)
{
	/// Automate name if none was given.
	if (child->name.Length() < 1)
		child->name = this->name + "Element"+String::ToString(childList.Size());

    /// Inherit neighbours.
	child->InheritNeighbours(this);

    std::cout<<"\nAdding child "<<child->name<<" text: "<<child->text;
	/// If first child, place it at the top.
	if (childList.Size() == 0){
		UIElement::AddChild(child);
		child->alignmentY = 1.0f - child->sizeRatioY * 0.5f;
		return;
	}
	// If not, find bottom child (or, bottom edge.!
	// Get bottom child
	float bottom = 1.0f;
	UIElement * bottomElement = NULL;
	for (int i = 0; i < childList.Size(); ++i){
		UIElement * c = childList[i];
		/// Skip 'system' elements like scroll-bars and handles that will be deleted shortly!
		if (c->isSysElement)
            continue;
		bottomElement = c;
	//	std::cout<<"\nBottomElement name: "<<bottomElement->name;
		float childBottomEdge = c->alignmentY - c->sizeRatioY * 0.5f;
		if (childBottomEdge < bottom)
			bottom = childBottomEdge;
	}
	UIElement::AddChild(child);
	child->alignmentY = bottom - child->sizeRatioY * 0.5f - padding;

	// Update total contents size.
	contentsSize = 1 - child->alignmentY + child->sizeRatioY * 0.5f;

	/// Check if we should add scroll-bars to zis list!
	if (child->alignmentY - child->sizeRatioY * 0.5f < 0){
	    if (scrollBarY){
	        childList.Remove(scrollBarY);
	   //     std::cout<<"\nTrying to delete stuff?";
			if (scrollBarY->vboBuffer != -1)
				scrollBarY->FreeBuffers();
	        if (scrollBarY->mesh)
                scrollBarY->DeleteGeometry();
	        delete scrollBarY;
	        scrollBarY = NULL;
	      //  scrollBarY->Update(bottom + child->sizeRatioY);
          //  scrollBarY->visible = true;
          //  FormatElements();
          //  return;
        }
        UIScrollBar * scroll = new UIScrollBar();
        scroll->CreateHandle();
        scroll->Update(1.0f - (bottom - child->sizeRatioY));
        scroll->sizeRatioX = 0.1f;
        scroll->alignmentX = 0.95f;
        scroll->textureSource = "img/80Gray50Alpha.png";
   //     scroll->text = "Neeeeej";
        scrollBarY = scroll;
        UIElement::AddChild(scrollBarY);
        FormatElements();
	//	assert(false && "Implement automatic scrollbars for your lists, yo!");
	}

	// Bind them for proper navigation.
    if (bottomElement){
	//	std::cout<<"\nBottomElement name: "<<bottomElement;

        bottomElement->downNeighbourName = child->name;
		child->upNeighbourName = bottomElement->name;
    }
}

#define RETURN_IF_OUTSIDE {if (mouseY > top || mouseY < bottom || \
                               mouseX > right || mouseX < left) return NULL;};

/// Activation functions
UIElement * UIList::Hover(float & mouseX, float & mouseY){
    RETURN_IF_OUTSIDE
    float listX = mouseX;
    float listY = mouseY;
    if (scrollBarY){
        listY -= scrollBarY->GetStart() * sizeY;
    }
    UIElement * e = NULL;
    /// Check le children.
    for (int i = 0; i < childList.Size(); ++i){
        UIElement * child = childList[i];
        if (child->isSysElement)
            e = child->Click(mouseX, mouseY);
        else
            e = child->Hover(listX, listY);
        if (e)
            break;
    }
    if (e == NULL)
        e = this;
    if (scrollBarY){
//        std::cout<<"\nUIList::Hover "<< (e ? e->name : "NULL") <<" listY: "<<listY<<" mouseY: "<<mouseY;
    }
    return e;
}
UIElement * UIList::Click(float & mouseX, float & mouseY){
    RETURN_IF_OUTSIDE
    float listX = mouseX;
    float listY = mouseY;
    if (scrollBarY)
        listY -= scrollBarY->GetStart() * sizeY;

    UIElement * e = NULL;
    /// Check le children.
    for (int i = 0; i < childList.Size(); ++i){
        UIElement * child = childList[i];
        if (child->isSysElement)
            e = child->Click(mouseX, mouseY);
        else
            e = child->Click(listX, listY);
        if (e)
            break;
    }
    if (!e)
        e = this;
    if (scrollBarY){
        std::cout<<"\nUIList::Click "<< (e ? e->name : "NULL") <<" listY: "<<listY<<" mouseY: "<<mouseY;
    }
    return e;
}
/// GEtttererrr
UIElement * UIList::GetElement(float & mouseX, float & mouseY){
    RETURN_IF_OUTSIDE
    float listX = mouseX;
    float listY = mouseY;
    if (scrollBarY)
        listY -= scrollBarY->GetStart() * sizeY;
    UIElement * e = this;
    /// Check le children.
    for (int i = 0; i < childList.Size(); ++i){
        UIElement * child = childList[i];
        if (child->isSysElement)
            e = child->Click(mouseX, mouseY);
        else
            e = child->GetElement(listX, listY);
        if (e)
            break;
    }
    if (!e)
        e = this;
    if (scrollBarY){
        std::cout<<"\nUIList::GetElement "<< (e ? e->name : "NULL") <<" listY: "<<listY<<" mouseY: "<<mouseY;
    }
    return e;
}


/// Scroll ze listur!
bool UIList::OnScroll(float delta){
    /// Move the slider and adjust content.
    if (scrollBarY){
        float pageSize = scrollBarY->handle->sizeRatioY;
        bool negative = delta < 0;
        float distance = AbsoluteValue(delta);
        float pixels = distance * sizeY;
        if (pixels < 5){
            distance = 5.0f / sizeY;
        }
        float quarterPage = pageSize * 0.25f;
        std::cout<<"\nQuarterPage: "<<quarterPage;
        if (distance > quarterPage){
            distance = quarterPage;
        }
        /// Fix sign.
        if (negative)
            distance *= -1.0f;
        scrollBarY->Move(distance);
    //    scrollBarY->PrintDebug();
    }
    return true;
}

/// Scroll, not capped.
bool UIList::Scroll(float absoluteDistanceInPages){
    /// Move the slider and adjust content.
    if (scrollBarY){
        float pageSize = scrollBarY->handle->sizeRatioY;
        float distance = absoluteDistanceInPages * pageSize;
        scrollBarY->Move(distance);
    //    scrollBarY->PrintDebug();
    }
    return true;
}

/// Adjusts positions and sizes acording to any attached scroll-bars or other system UI elements.
void UIList::FormatElements(){
    if (!formatX)
        return;
    for (int i = 0; i < childList.Size(); ++i){
        UIElement * e = childList[i];
        if (e->isSysElement)
            continue;
        if (scrollBarY){
            if (e->sizeRatioX > 1.0f - scrollBarY->sizeRatioX){
                float newSizeRatioX = 1.0f - scrollBarY->sizeRatioX;
                e->alignmentX = e->alignmentX + (newSizeRatioX - e->sizeRatioX) * 0.5f;
                e->sizeRatioX = newSizeRatioX;
            }
        }
    }
}

/// Rendering
void UIList::Render(GraphicsState& graphics){
    // Push matrices
    Matrix4d tmp = graphics.modelMatrixD;

    // Render ourself and maybe children.
    RenderSelf(graphics);
    if (childList.Size())
        RenderChildren(graphics);

    // Pop matrices
    graphics.modelMatrixF = graphics.modelMatrixD = tmp;

}

void UIList::RenderChildren(GraphicsState & graphics){

    Matrix4d modelMatrix = graphics.modelMatrixD;
    Matrix4d translatedModelMatrix = graphics.modelMatrixD;

    /// If we got a scrollbar, apply a model matrix to make us render at the proper location.
    if (scrollBarY && scrollBarY->visible){
        float pageBegin = scrollBarY->GetStart();
   //     std::cout<<"\nPage Begin (relative): "<<pageBegin;
        pageBegin *= sizeY;
   //     std::cout<<" (pixels): "<<pageBegin;
        translatedModelMatrix *= Matrix4d::Translation(0, pageBegin, 0);
    }

    /// Render all children
	for (int i = 0; i < childList.Size(); ++i){
        UIElement * child = childList[i];
        if (!child->visible)
            continue;

        /// Set model matrix depending on child-type.
        if (child->isSysElement){
        //    std::cout<<"\nChild: "<<child->name<<" is sys element.";
            graphics.modelMatrixF = graphics.modelMatrixD = modelMatrix;
        }
        else
            graphics.modelMatrixF = graphics.modelMatrixD = translatedModelMatrix;

// 		if (name == "DebugList"){
// 			std::cout<<"\nScissor: l:"<<graphics.leftScissor<<" r:"<<graphics.rightScissor<<" t:"<<graphics.topScissor<<" b:"<<graphics.bottomScissor;
// 			std::cout<<"This l:"<<left<<" r:"<<right<<" t:"<<top<<" b:"<<bottom;
// 		}

        Rect previousScissor((int)graphics.leftScissor, (int)graphics.bottomScissor, (int)graphics.rightScissor, (int)graphics.topScissor);
	    graphics.leftScissor = graphics.leftScissor > left ? graphics.leftScissor : left;
	    graphics.rightScissor = graphics.rightScissor < right ? graphics.rightScissor : right;
	    graphics.bottomScissor = graphics.bottomScissor > bottom ? graphics.bottomScissor : bottom;
	    graphics.topScissor = graphics.topScissor < top ? graphics.topScissor : top;

	    int scissorWidth = (int) (graphics.rightScissor - graphics.leftScissor);
	    int scissorHeight = (int) (graphics.topScissor - graphics.bottomScissor);
	  //  std::cout<<"\nScissor e "<<name<<" left: "<<left<<" right: "<<right<<" top: "<<top<<" bottom: "<<bottom<<" width: "<<scissorWidth<<" height: "<<scissorHeight;
	  //  assert(scissorWidth >= 0);
	  //  assert(scissorHeight >= 0);
	  /// If bad scissor do nothing.
        if (scissorWidth < 0 || scissorHeight < 0){

        }
        /// If not, render.
        else {
            bool scissorDisabled = (graphics.settings & SCISSOR_DISABLED) > 0;
            if (!scissorDisabled){
             //   std::cout<<"\nGLScissor: e "<<name<<" posX "<<posX<<" sizeX "<<sizeX<<" posY "<<posY<<" sizeY "<<sizeY;
				glScissor((GLint) (graphics.leftScissor + graphics.viewportX0),
					(GLint) (graphics.bottomScissor + graphics.viewportY0),
					scissorWidth,
					scissorHeight);
            }
            child->Render(graphics);
        }
        /// And pop the scissor statistics back as they were.
		graphics.leftScissor = (float)previousScissor.x0;
	    graphics.rightScissor = (float)previousScissor.x1;
	    graphics.bottomScissor = (float)previousScissor.y0;
	    graphics.topScissor = (float)previousScissor.y1;
	}
}

UIColumnList::UIColumnList(String name)
: UIElement()
{
	this->name = name;
    type = UIType::COLUMN_LIST;
}

UIColumnList::~UIColumnList()
{
	std::cout<<"\nUIColumnList destructor.";
}

/// Deletes all children and content inside.
void UIColumnList::Clear(){
	while(childList.Size()){
		UIElement * c = childList[0];
		childList.Remove(c);
		c->FreeBuffers();
		c->DeleteGeometry();
		delete c;
		c = NULL;
	}
}
// Adjusts hierarchy besides the regular addition
void UIColumnList::AddChild(UIElement* child)
{
	/// Automate name if none was given.
	if (child->name.Length() < 1)
		child->name = this->name + "Element"+String::ToString(childList.Size());

    /// Inherit up- and down- neighbours.
	child->InheritNeighbours(this);

	/// If first child, place it at the top.
	if (childList.Size() == 0){
		UIElement::AddChild(child);
		child->alignmentX = 0.0f + child->sizeRatioX * 0.5f + padding;
		return;
	}
	// If not, find bottom child (or, bottom edge.!
	// Get bottom child
	float left = 0.f;
    // Bind UIs too, so save the rightmost child within for later.
    UIElement * rightmost = NULL;
	for (int i = 0; i < childList.Size(); ++i){
		UIElement * c = childList[i];
		rightmost = c;
		float childLeftEdge = c->alignmentX + c->sizeRatioX * 0.5f;
		if (childLeftEdge > left)
			left = childLeftEdge;
	}
	UIElement::AddChild(child);
	child->alignmentX = left + child->sizeRatioX * 0.5f + padding;

    // Bind them for proper navigation.
    if (rightmost){
        rightmost->rightNeighbourName = child->name;
        child->leftNeighbourName = rightmost->name;
    }

	/// Check if we should add scroll-bars to zis list!
	if (child->alignmentX + child->sizeRatioX * 0.5f > 1.0f){
		// assert(false && "Implement automatic scrollbars for your columns lists, yo!");
		std::cout<<"\nImplement automatic scrollbars for your columns lists, yo!";
	}
}


/// Called to ensure visibility of target element.
void UIList::EnsureVisibility(UIElement * element){
	if (!element)
		return;
	// Check if we need to do anything at all (if small list)
	if (contentsSize <= 1.01f)
		return;

	// Do stuff! o.o
	float alignmentY = 1 - element->alignmentY;
//	std::cout<<"\nAlignmentY: "<<alignmentY;
	float halfSizeY = element->sizeRatioY * 0.5f;
	float top = alignmentY - halfSizeY;
	float bottom = alignmentY + halfSizeY;

	// Extract positions for this element.
	float pageTop = this->scrollBarY->GetStart();
	float pageStop = this->scrollBarY->GetStop();

	// If elements top exceeding current viewable top, scroll up.
	if (top <= pageTop){
		float dist = pageTop - top;
		this->Scroll(dist);
		std::cout<<"\nScrolling up.";
	}
	// And if element's bottom exceeding current viewable bottom, scroll down..!
	else if (bottom >= pageStop){
		float dist = pageStop - bottom;
		this->Scroll(dist);
		std::cout<<"\nScrolling down.";
	}

//	float


	// Compare with scroll-bar position.

	//this->Scroll(-0.1f);
}
