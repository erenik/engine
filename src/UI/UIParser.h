/// Emil Hedemalm
/// 2020-08-12
/// Parser for UI, elements and their tags.

#pragma once 

#include "UIElement.h"

#define ENSURE_NEXT_TOKEN EnsureNextToken(tokens);

#define NEXT_TOKEN	(NextToken(tokens)) 

#define SET_DEFAULTS SetDefaults(element);


class UIParser 
{
public:

	UIParser();

	/// Default values that can be set when parsing
	int defaultAlignment = UIElement::NULL_ALIGNMENT;
	String defaultTexture = "default.png";
	String defaultParent = "root";
	String defaultRootFolder = "";
	String defaultTopBorder = "", defaultRightBorder = "";
	String defaultTopRightCorner = "";
	bool defaultScalability = true;
	bool defaultVisibility = true;
	bool defaultExitability = true;
	Vector4f defaultTextColor = Vector4f(0, 0, 0, 1);
	float defaultSizeRatioY = 1.0f;
	float defaultSizeRatioX = 1.0f;
	float defaultPadding = 0.0f;
	float defaultTextSize = 1.0f;
	String defaultOnTrigger = "";
	Vector2f defaultDivider = Vector2f(0.5f, 0.5f);
	int defaultTextAlignment = UIElement::LEFT;

	/// Manually parse the line using a few identifiers that can be relevant.
	int lastEvaluatedIndex = 0;
	List<char> stack;
	char last;
	char cChar;
	String line;

	/// Loads from target file, using given root as root-element in the UI-hierarchy.
	UIElement* LoadFromFile(String filePath, UserInterface * ui);

	List<String> ParseTokens(String fromLine);

	void EnsureNextToken(const List<String> fromTokens);
	String NextToken(const List<String> fromTokens);

	void SetDefaults(UIElement * forElement);

	void AddPreviousToUIIfNeeded();

private:
	UIElement * root;
	UIElement * element;

};

