/// Emil Hedemalm
/// 2014-08-25
/// Lazy include file for all Input-specific UI elements, not including 
/// file-browsers or advanced inputs such as matrices.

// Base input class.
#include "Input/UIInput.h"
/// Aggregate types which leverage the UIInputMan.
#include "Input/UIFloatInput.h"
#include "Input/UIIntegerInput.h"
#include "Input/UIVectorInput.h"
#include "Input/UIStringInput.h"
#include "Input/UITextureInput.h"
/// Long text field, not sure if it should stay here.
#include "Input/UITextField.h"
/// Drop-down and menu-aggregates
#include "Input/UIDropDownMenu.h"


