/// Emil Hedemalm
/// 2020-08-12
/// Parser for UI, elements and their tags.

#include "UIParser.h"
#include "Graphics/Fonts/TextFont.h"
#include "File/LogFile.h"
#include <fstream>
#include "String/StringUtil.h"

#include "UILists.h"
#include "UIBar.h"
#include "UIInputs.h"
#include "UIButtons.h"
#include "UIVideo.h"
#include "UIImage.h"
#include "UILog.h"
#include "UI/UITypes.h"
#include "UI/DataUI/UIMatrix.h"
#include "UI/Buttons/UIRadioButtons.h"
#include "UI/Buttons/UICheckbox.h"
#include <iomanip>

// Parsing stuff

#define ADD_PREVIOUS_TO_UI_IF_NEEDED AddPreviousToUIIfNeeded();

UIParser::UIParser()
	: defaultAlignment(UIElement::NULL_ALIGNMENT)
	, defaultTexture ("default.png")
	, defaultParent ("root")
	, defaultRootFolder ("")
	, defaultTopBorder ("")
	, defaultRightBorder ("")
	, defaultTopRightCorner ( "")
	, defaultScalability ( true)
	, defaultVisibility ( true)
	, defaultExitability ( true)
	, defaultTextColor ( Vector4f(0, 0, 0, 1))
	, defaultSizeRatioY ( 1.0f)
	, defaultSizeRatioX ( 1.0f)
	, defaultPadding ( 0.0f)
	, defaultTextSize ( 1.0f)
	, defaultOnTrigger ( "")
	, defaultDivider ( Vector2f(0.5f, 0.5f))
	, defaultTextAlignment ( UIElement::LEFT)
	, lastEvaluatedIndex ( 0)
	, element(nullptr)
	, root(nullptr)
{

}


/// Loads from target file, using given root as root-element in the UI-hierarchy.
UIElement* UIParser::LoadFromFile(String filePath, UserInterface * ui){
		
	root = new UIElement();
	root->SetRootDefaults(ui);

	char * data;
	int fileSize;
	std::fstream file;
	file.open(filePath.c_str(), std::ios_base::in);

	//	assert(file.is_open() && "Unable to open file in AppState::LoadUI");
	if (!file.is_open()) {
		std::cout << "\nUserInterface::LoadFromFile: Unable to open file: " << filePath;
		return false;
	}
	root->source = filePath;

	// Get size by seeking to end of file
	int start = (int)file.tellg();
	file.seekg(0, std::ios::end);
	fileSize = (int)file.tellg();

	// Allocate data array to required length
	data = new char[fileSize + 5];
	memset(data, 0, fileSize + 5);

	// Go to beginning of file and read the data
	file.seekg(0, std::ios::beg);
	file.read((char*)data, fileSize);
	// Close file stream
	file.close();

	assert(!file.bad());

	LogMain("=====================================\nBeginning parsing file " + filePath, INFO);

	// Dump data into handable format.
	String contents;
	contents = data;

	// Delete data
	delete[] data;
	data = NULL;

	List<String> lines = contents.GetLines();

	for (int i = 0; i < lines.Size(); ++i) {
		;//  std::cout<<"\nLine "<<i<<lines[i];
	}

	String str;
	enum parsingState {
		NULL_STATE,
		MID_COMMENT,	 // For /* */
	};
	int parsingState = NULL_STATE;



	/// Read until done or too many errors!
	bool wasLastLine = false;
	LogMain("Lines to parse: " + String(lines.Size()), INFO);
	Timer parseStart;
	parseStart.Start();
	for (int i = 0; i < lines.Size(); ++i) {

		line = lines[i];
		if (line.Length() < 1)
			continue;

		/// Manually parse the line using a few identifiers that can be relevant.
		lastEvaluatedIndex = 0;
		UIParser::stack.Clear();

		List<String> tokens = ParseTokens(line);

		List<String> newTokens = TokenizeIgnore(line, " \n\r\t", "\"");
		if (tokens.Size() == 0)
			tokens = newTokens;

		List<String> strings = line.Tokenize("\"");
		String firstQuote, secondQuote, thirdQuote;
		if (strings.Size() >= 2)
			firstQuote = strings[1];
		else if (tokens.Size() >= 2)
			firstQuote = tokens[1];

		if (strings.Size() >= 3)
			secondQuote = strings[2];
		else if (tokens.Size() >= 3)
			secondQuote = tokens[2];

		if (strings.Size() >= 4)
			thirdQuote = strings[3];
		if (tokens.Size() >= 4)
			thirdQuote = tokens[3];

		if (tokens.Size() < 1)
			continue;

		String value;
		if (tokens.Size() > 1)
			value = tokens[1];

		for (int t = 0; t < tokens.Size(); ++t) {
			String token = tokens[t];
			token.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			//		for (int i = 0; i < token.Length(); ++i)
			  //          std::cout<<"\n"<<i<<": (int)"<<(int)token.c_str()[i]<<" (char)"<<token.c_str()[i];
			if (token.Contains("\r")) {
				//      std::cout<<"Token '\\r'! Skipping o-o";
				continue;
			}
			// Evaluate some things first depending on the current parsing state
			else if (parsingState == MID_COMMENT) {
				if (token.Contains("*/")) {
					parsingState = NULL_STATE;
					continue;
				}
				continue;
			}
			// Regular parses
			else if (token.Contains("//")) {
				// Skip the rest of the line
				// Done by default at the end of these if-elseif-clauses
				break;
			}
			else if (token.Contains("/*")) {
				parsingState = MID_COMMENT;
				continue;
			}
			else if (token == "defaultAlignment") {
				ENSURE_NEXT_TOKEN
					defaultAlignment = UIElement::GetAlignment(NEXT_TOKEN);
			}
			else if (token == "defaultTextAlignment")
			{
				ENSURE_NEXT_TOKEN
					defaultTextAlignment = UIElement::GetAlignment(value);
			}
			else if (token == "defaultTexture") {
				ENSURE_NEXT_TOKEN
					String param = tokens[1];
				param.SetComparisonMode(String::NOT_CASE_SENSITIVE);
				if (param == "NULL")
					defaultTexture = String();
				else
					defaultTexture = param;
			}
			else if (token == "defaultTopBorder") {
				ENSURE_NEXT_TOKEN;
				defaultTopBorder = (line - "defaultTopBorder");
				defaultTopBorder.RemoveSurroundingWhitespaces();
			}
			else if (token == "defaultRightBorder") {
				ENSURE_NEXT_TOKEN;
				defaultRightBorder = (line - "defaultRightBorder");
				defaultRightBorder.RemoveSurroundingWhitespaces();
			}
			else if (token == "defaultTopRightCorner") {
				ENSURE_NEXT_TOKEN;
				defaultTopRightCorner = (line - "defaultTopRightCorner");
				defaultTopRightCorner.RemoveSurroundingWhitespaces();
			}
			else if (token == "defaultOnTrigger") {
				ENSURE_NEXT_TOKEN
					defaultOnTrigger = NEXT_TOKEN;
			}
			else if (token == "defaultParent" ||
				token == "parent")
			{
				ADD_PREVIOUS_TO_UI_IF_NEEDED
					ENSURE_NEXT_TOKEN
					defaultParent = NEXT_TOKEN;
			}
			else if (token == "defaultScalability") {
				ENSURE_NEXT_TOKEN
					defaultScalability = NEXT_TOKEN.ParseBool();
			}
			else if (token == "defaultExitability")
			{
				defaultExitability = value.ParseBool();
			}
			else if (token == "defaultDividerX")
			{
				EnsureNextToken(tokens);
				defaultDivider.x = NextToken(tokens).ParseFloat();
			}
			else if (token == "defaultVisibility") {
				ENSURE_NEXT_TOKEN
					defaultVisibility = NEXT_TOKEN.ParseBool();
			}
			else if (token == "defaultSizeRatioXY" ||
				token == "defaultSizeRatio" ||
				token == "defaultSizeXY" ||
				token == "defaultSize")
			{
				if (tokens.Size() == 2) {
					defaultSizeRatioX = defaultSizeRatioY = tokens[1].ParseFloat();
				}
				else if (tokens.Size() >= 3) {
					defaultSizeRatioX = tokens[1].ParseFloat();
					defaultSizeRatioY = tokens[2].ParseFloat();
				}
			}
			else if (token == "defaultSizeRatioY") {
				ENSURE_NEXT_TOKEN
					defaultSizeRatioY = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "defaultSizeRatioX") {
				ENSURE_NEXT_TOKEN
					defaultSizeRatioX = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "defaultPadding") {
				ENSURE_NEXT_TOKEN
					defaultPadding = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "defaultTextSize") {
				ENSURE_NEXT_TOKEN
					defaultTextSize = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "defaultTextColor")
			{
				// Hex detected!
				if (line.Contains("0x"))
				{
					defaultTextColor = Color::ColorByHexName(NEXT_TOKEN);
				}
				else
				{
					switch (tokens.Size() - 1)
					{
					case 1: // Assume it's alpha and keep the other colors as usual
						defaultTextColor[3] = NEXT_TOKEN.ParseFloat();
						break;
					case 4:
						defaultTextColor[3] = tokens[4].ParseFloat();
					case 3: // Assume it's RGB
						defaultTextColor[0] = tokens[1].ParseFloat();
						defaultTextColor[1] = tokens[2].ParseFloat();
						defaultTextColor[2] = tokens[3].ParseFloat();
						break;
					case 2: case 0:
						assert(false && "Irregular amount of tokens following \"defaultTextColor\"; 1 for alpha, 3 for RGB and 4 for RGBA.");
						break;
					}
				}
			}
			else if (token == "defaultRootFolder") {
				ENSURE_NEXT_TOKEN
					defaultRootFolder = NEXT_TOKEN + "/";
				if (NEXT_TOKEN == "NULL")
					defaultRootFolder = "";
			}
			else if (token == "root") {
				if (tokens.Size() > 1) {
					delete root;
					String typeName = tokens[1];
					if (typeName == "ToggleButton") {
						root = new UIToggleButton();
						root->SetRootDefaults(ui);
					}
					else
						assert(false && "Unsupported root-type");
				}
				element = root;
			}
			else if (token == "element" || token == "div") {
				ADD_PREVIOUS_TO_UI_IF_NEEDED
					element = new UIElement();
				if (tokens.Size() > 1)
					element->name = firstQuote;
				SET_DEFAULTS
			}
			else if (token == "Button") {
				ADD_PREVIOUS_TO_UI_IF_NEEDED
					element = new UIButton();
				if (tokens.Size() > 1) {
					element->name = firstQuote;
					/// Set the elements text and message default to it's name too, yo.
					element->activationMessage = element->text = element->name;
				}
				SET_DEFAULTS
			}
			else if (token == "Label") {
				ADD_PREVIOUS_TO_UI_IF_NEEDED
					element = new UILabel();
				if (tokens.Size() > 1) {
					element->name = tokens[1];
					element->SetText(firstQuote);
				}
				SET_DEFAULTS
			}
			else if (token == "TextField" || token == "Input")
			{
				ADD_PREVIOUS_TO_UI_IF_NEEDED
					element = new UITextField();
				SET_DEFAULTS
					if (tokens.Size() > 1)
						element->name = element->onTrigger = firstQuote;
			}
			else if (token == "List") {
				ADD_PREVIOUS_TO_UI_IF_NEEDED
					element = new UIList();
				if (tokens.Size() > 1)
					element->name = firstQuote;
				SET_DEFAULTS
			}
			else if (token == "ColumnList") {
				ADD_PREVIOUS_TO_UI_IF_NEEDED
					element = new UIColumnList();
				if (tokens.Size() > 1)
					element->name = firstQuote;
				SET_DEFAULTS
			}
			else if (token == "Log")
			{
				ADD_PREVIOUS_TO_UI_IF_NEEDED
					element = new UILog();
				if (tokens.Size() > 1)
					element->name = firstQuote;
				SET_DEFAULTS
			}
			else if (token == "checkbox") {
				AddPreviousToUIIfNeeded();
				String firstToken = tokens[1];
				UICheckbox * checkBox = new UICheckbox(firstToken);
				checkBox->displayText = firstQuote;
				checkBox->CreateChildren();
				if (line.Contains("Checked"))
					checkBox->SetToggled(true);
				element = checkBox;

				SET_DEFAULTS
			}
			else if (token == "DropDownMenu")
			{
				ADD_PREVIOUS_TO_UI_IF_NEEDED;
				UIDropDownMenu * ddm = new UIDropDownMenu(firstQuote);
				element = ddm;
				SET_DEFAULTS;
				ddm->CreateChildren(nullptr);
			}
			else if (token == "Matrix")
			{
				ADD_PREVIOUS_TO_UI_IF_NEEDED;
				UIMatrix * matrix = new UIMatrix(firstQuote);
				element = matrix;
				SET_DEFAULTS;
				matrix->CreateChildren(nullptr);
			}
			else if (token == "TextureInput")
			{
				ADD_PREVIOUS_TO_UI_IF_NEEDED;
				String firstToken = tokens[1];
				UITextureInput * ti = new UITextureInput(firstToken, "Set" + firstToken);
				element = ti;
				element->displayText = firstQuote;
				SET_DEFAULTS;
				ti->CreateChildren(nullptr);
			}
			else if (token == "StringInput")
			{
				ADD_PREVIOUS_TO_UI_IF_NEEDED;
				String firstToken = tokens[1];
				UIStringInput * si = new UIStringInput(firstToken, "Set" + firstToken);
				element = si;
				element->displayText = firstQuote;
				SET_DEFAULTS;
			}
			else if (token == "StringValue")
			{
				if (!element)
					continue;
				if (element->type == UIType::STRING_INPUT)
				{
					UIStringInput * si = (UIStringInput*)element;
					si->SetText(firstQuote);
				}
			}
			else if (token == "IntegerInput" ||
				token == "IntInput")
			{
				ADD_PREVIOUS_TO_UI_IF_NEEDED;
				String firstToken = tokens[1];
				UIIntegerInput * ii = new UIIntegerInput(firstToken, "Set" + firstToken);
				element = ii;
				element->displayText = firstQuote;
				SET_DEFAULTS
			}
			else if (token == "IntegerLabel") // Creates an Integer-display which is not interactable via GUI, just for display.
			{
				ADD_PREVIOUS_TO_UI_IF_NEEDED;
				String firstToken = tokens[1];
				UIIntegerInput * ii = new UIIntegerInput(firstToken, "Set" + firstToken);
				element = ii;
				element->displayText = firstQuote;
				element->hoverable = element->activateable = false;
				SET_DEFAULTS
					ii->guiInputDisabled = true;
			}
			else if (token == "StringLabel") // Creates an String-display which is not interactable via GUI, just for display.
			{
				ADD_PREVIOUS_TO_UI_IF_NEEDED;
				String firstToken = tokens[1];
				UIStringInput * si = new UIStringInput(firstToken, "Set" + firstToken);
				element = si;
				element->displayText = firstQuote;
				element->hoverable = element->activateable = false;
				SET_DEFAULTS;
				si->guiInputDisabled = true;
			}
			else if (token == "FloatInput") {
				ADD_PREVIOUS_TO_UI_IF_NEEDED;
				String firstToken = tokens[1];
				UIFloatInput * fi = new UIFloatInput(firstToken, "Set" + firstToken);
				element = fi;
				element->displayText = firstQuote;
				SET_DEFAULTS
			}
			else if (token == "FloatLabel") {
				AddPreviousToUIIfNeeded();
				String firstToken = tokens[1];
				UIFloatInput * fi = new UIFloatInput(firstToken, "Set" + firstToken);
				element = fi;
				element->displayText = firstQuote;
				element->hoverable = element->activateable = false;
				//fi->guiInputDisabled = true;
				SetDefaults(element);
			}
			else if (token == "FloatValue")
			{
				if (!element)
					continue;
				if (element->type == UIType::FLOAT_INPUT)
				{
					UIFloatInput * fi = (UIFloatInput*)element;
					fi->SetValue(firstQuote.ParseFloat());
				}
			}
			else if (token == "VectorInput") {
				ADD_PREVIOUS_TO_UI_IF_NEEDED
					String action = "Set" + secondQuote;
				UIVectorInput * vi = new UIVectorInput(firstQuote.ParseInt(), secondQuote, action);
				element = vi;
				SET_DEFAULTS
			}
			else if (token == "RadioButtons")
			{
				ADD_PREVIOUS_TO_UI_IF_NEEDED

					String name, displayText;
				int numItems = 1;
				if (newTokens.Size() >= 3)
				{
					numItems = newTokens[1].ParseInt();
					name = newTokens[2];
				}
				if (newTokens.Size() >= 4)
				{
					displayText = newTokens[3];
				}
				else
					displayText = name;
				String action = "Set" + name;
				UIRadioButtons * rb = new UIRadioButtons(numItems, name, action);
				element = rb;
				displayText.Remove('\"', true);
				rb->displayText = displayText;
				SET_DEFAULTS
			}
			else if (token == "Image")
			{
				ADD_PREVIOUS_TO_UI_IF_NEEDED
					UIImage * image = new UIImage(firstQuote, secondQuote);
				element = image;
				SET_DEFAULTS
					if (secondQuote.Length())
						image->textureSource = secondQuote;
			}
			else if (token == "Bar") {
				ADD_PREVIOUS_TO_UI_IF_NEEDED
					UIBar * bar = new UIBar(firstQuote);
				element = bar;
				SET_DEFAULTS
			}
			else if (token == "Video") {
				ADD_PREVIOUS_TO_UI_IF_NEEDED
					UIVideo * video = new UIVideo(firstQuote, secondQuote);
				element = video;
				SET_DEFAULTS
					video->CreateStream();
			}
			/// Single expressions that apply effects to an element
			else if (token == "navigateUIOnPush") {
				element->navigateUIOnPush = true;
			}
			else if (token == "disabled")
			{
				element->Disable();
			}
			else if (token == "NoLabel")
			{
				element->noLabel = true;
			}
			/// All expressions requiring arguments below!
			else if (token.Length() <= 2)
				break;
			else if (tokens.Size() < 2) {
				std::cout << "\nINFO: Lacking argument on line " << i << ": " << line;
				std::cout << "\nSkipping row and going to next one!";
				break;
			}
			else if (token == "DataType")
			{
				switch (element->type)
				{
				case UIType::VECTOR_INPUT:
				{
					UIVectorInput * vi = (UIVectorInput*)element;
					String dataType = NEXT_TOKEN;
					dataType.SetComparisonMode(String::NOT_CASE_SENSITIVE);
					if (dataType.Contains("Integer"))
						vi->SetDataType(UIVectorInput::INTEGERS);
					else
						vi->SetDataType(UIVectorInput::FLOATS);
					break;
				}
				}
			}
			else if (element == NULL) {
				std::cout << "\nTrying to act upon a null-element! Continuing until a new element is created!";
				break;
			}
			else if (token == "Name") {
				ENSURE_NEXT_TOKEN
					element->name = firstQuote;
			}
			else if (token == "displayText")
			{
				ENSURE_NEXT_TOKEN
					element->displayText = firstQuote;
			}
			else if (token == "Padding") {
				ENSURE_NEXT_TOKEN
					element->padding = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "OnActivate") {
				ENSURE_NEXT_TOKEN
					element->activationMessage = NEXT_TOKEN;
			}
			else if (token == "onTrigger") {
				ENSURE_NEXT_TOKEN
					element->onTrigger = NEXT_TOKEN;
			}
			else if (token == "removeOnPop")
			{
				ENSURE_NEXT_TOKEN
					element->removeOnPop = NEXT_TOKEN.ParseBool();
			}
			else if (token == "onEnterScope")
			{
				ENSURE_NEXT_TOKEN
					element->onEnterScope = NEXT_TOKEN;
			}
			else if (token == "onPop")
			{
				ENSURE_NEXT_TOKEN
					element->onPop = NEXT_TOKEN;
			}
			else if (token == "onExit") {
				ENSURE_NEXT_TOKEN
					element->onExit = NEXT_TOKEN;
			}
			else if (token == "exitable") {
				ENSURE_NEXT_TOKEN
					element->exitable = (NEXT_TOKEN).ParseBool();
			}
			else if (token == "hoverable")
			{
				ENSURE_NEXT_TOKEN;
				element->hoverable = NEXT_TOKEN.ParseBool();
			}
			else if (token == "activatable") {
				EnsureNextToken(tokens);
				element->activateable = NextToken(tokens).ParseBool();
			}
			else if (token == "deleteOnPop") {
				EnsureNextToken(tokens);
				element->deleteOnPop = NextToken(tokens).ParseBool();
			}
			else if (token == "highlightOnHover")
			{
				EnsureNextToken(tokens);
				element->highlightOnHover = NextToken(tokens).ParseBool();
			}
			else if (token == "LabelText")
			{
				ENSURE_NEXT_TOKEN
					element->labelText = firstQuote;
			}
			else if (token == "visible" || token == "visibility")
			{
				ENSURE_NEXT_TOKEN
					element->visible = (NEXT_TOKEN).ParseBool();
			}
			else if (token == "maxDecimals") {
				ENSURE_NEXT_TOKEN;
				switch (element->type)
				{
				case UIType::FLOAT_INPUT:
				{
					UIFloatInput * fi = (UIFloatInput *)element;
					fi->maxDecimals = NEXT_TOKEN.ParseInt();
					break;
				}
				}
			}
			else if (token == "Range") {
				EnsureNextToken(tokens);
				UIIntegerInput * ii = (UIIntegerInput*)element;
				ii->SetRange(tokens[1].ParseInt(), tokens[2].ParseInt());
			}
			else if (token == "texture") {
				ENSURE_NEXT_TOKEN
					String param = firstQuote;
				//				param.Remove("\"", true);
				param.SetComparisonMode(String::NOT_CASE_SENSITIVE);
				if (element->type == UIType::RADIO_BUTTONS)
				{
					UIRadioButtons * radio = (UIRadioButtons*)element;
					radio->SetTextureSource(param);
				}
				if (param == "NULL")
					element->textureSource = String();
				else
					element->textureSource = defaultRootFolder + param;
			}
			else if (token == "text")
			{
				ENSURE_NEXT_TOKEN
					String text = tokens[1];
				/*		std::cout<<"\nTex: "<<line;
						for (int i = 0; i < line.Length(); ++i){
							std::cout<<"\nc: (int)"<<(int)line.c_str()[i]<<" char: "<<(char)line.c_str()[i];
						} */
						//        std::cout<<"\nLine: "<<line;

				List<String> tmp = line.Tokenize("\"");
				if (tmp.Size() > 1)
					text = tmp[1];
				else
					text = Text();
				/// Check special cases like dedicated label child elements.
				if (element->label)
				{
					element->label->SetText(text);
					element->SetText("");
					break;
				}
				element->SetText(text);
			}
			/// Used for setting aggregate types. 
			else if (token == "texts")
			{
				List<String> texts = line.Tokenize("\"");
				/// Removing each other index should gather all texts appropriately?
				texts.RemoveIndex(0, ListOption::RETAIN_ORDER);
				for (int i = 1; i < texts.Size(); ++i)
				{
					texts.RemoveIndex(i, ListOption::RETAIN_ORDER);
				}
				// First off: UIRadioButtons
				switch (element->type)
				{
				case UIType::RADIO_BUTTONS:
				{
					UIRadioButtons * rb = (UIRadioButtons*)element;
					rb->SetTexts(texts);
					break;
				}
				}

			}
			else if (token == "textColor")
			{
				// Hex detected!
				if (line.Contains("0x"))
				{
					element->text.color = Color::ColorByHexName(NEXT_TOKEN);
				}
				else
				{
					switch (tokens.Size() - 1)
					{
					case 1: // Assume it's alpha and keep the other colors as usual
						element->text.color[3] = NEXT_TOKEN.ParseFloat();
						break;
					case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11: case 12:
						element->text.color[3] = tokens[4].ParseFloat();
					case 3: // Assume it's RGB
						element->text.color[0] = tokens[1].ParseFloat();
						element->text.color[1] = tokens[2].ParseFloat();
						element->text.color[2] = tokens[3].ParseFloat();
						break;
					case 2: case 0:
						assert(false && "Irregular amount of tokens following \"textColor\"; 1 for alpha, 3 for RGB and 4 for RGBA.");
						break;
					}
				}
			}
			else if (token == "textSizeRatio" || token == "textSize") {
				ENSURE_NEXT_TOKEN
					element->textSizeRatio = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "textAlignment")
			{
				value.SetComparisonMode(String::NOT_CASE_SENSITIVE);
				if (value == "Center")
				{
					element->textAlignment = UIElement::CENTER;
				}
				else if (value == "Right") {
					element->textAlignment = UIElement::RIGHT;
				}
				else if (value == "Left") {
					element->textAlignment = UIElement::LEFT;
				}
			}
			else if (token == "dividerX")
			{
				EnsureNextToken(tokens);
				element->divider.x = NextToken(tokens).ParseFloat();
			}
			else if (token == "textPadding") {
				EnsureNextToken(tokens);
				element->textPaddingPixels = NextToken(tokens).ParseInt();
			}
			else if (token == "origin") {
				ENSURE_NEXT_TOKEN
					element->origin = NEXT_TOKEN.ParseInt();
			}
			else if (token == "scalable") {
				ENSURE_NEXT_TOKEN
					element->scalable = NEXT_TOKEN.ParseBool();
			}
			else if (token == "formatX") {
				ENSURE_NEXT_TOKEN
					element->formatX = NEXT_TOKEN.ParseBool();
			}
			else if (token == "lineSizeRatio")
			{
				ENSURE_NEXT_TOKEN
					element->lineSizeRatio = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "sizeRatioX") {
				ENSURE_NEXT_TOKEN
					element->sizeRatioX = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "sizeRatioY") {
				ENSURE_NEXT_TOKEN
					element->sizeRatioY = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "sizeRatio") {
				ENSURE_NEXT_TOKEN
					element->sizeRatioX = element->sizeRatioY = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "sizeRatioXY") {
				if (tokens.Size() < 3)
					continue;
				element->sizeRatioX = tokens[1].ParseFloat();
				element->sizeRatioY = tokens[2].ParseFloat();
			}
			else if (token == "alignment") {
				ENSURE_NEXT_TOKEN
					element->alignment = UIElement::GetAlignment(NEXT_TOKEN);
			}
			else if (token == "retainAspectRatio") {
				EnsureNextToken(tokens);
				element->retainAspectRatioOfTexture = NextToken(tokens).ParseBool();
			}
			else if (token == "maxWidth") {
				ENSURE_NEXT_TOKEN;
				element->maxWidth = tokens[1].ParseInt();
			}
			else if (token == "alignmentX") {
				ENSURE_NEXT_TOKEN
					element->alignmentX = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "topBorder") {
				ENSURE_NEXT_TOKEN;
				element->topBorderTextureSource = (line - "topBorder");
				element->topBorderTextureSource.RemoveSurroundingWhitespaces();
			}
			else if (token == "rightBorder") {
				ENSURE_NEXT_TOKEN;
				element->rightBorderTextureSource = (line - "rightBorder");
				element->rightBorderTextureSource.RemoveSurroundingWhitespaces();
			}
			else if (token == "topRightCorner") {
				ENSURE_NEXT_TOKEN;
				element->topRightCornerTextureSource = (line - "topRightCorner");
				element->topRightCornerTextureSource.RemoveSurroundingWhitespaces();
			}
			else if (token == "textures") {
				ENSURE_NEXT_TOKEN;
				UIType type = element->type;
				switch (type) {
				case UIType::RADIO_BUTTONS:
					UIRadioButtons* radioButtons = (UIRadioButtons*)element;
					List<String> textures = tokens;
					textures.RemoveIndex(0, ListOption::RETAIN_ORDER);
					radioButtons->SetTextures(textures);
				}
			}
			else if (token == "alignmentY") {
				ENSURE_NEXT_TOKEN
					element->alignmentY = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "alignmentXY" || token == "alignment") {
				if (tokens.Size() < 3)
					continue;
				element->alignmentX = tokens[1].ParseFloat();
				element->alignmentY = tokens[2].ParseFloat();
				int b = element->alignmentY + 3;
			}
			else if (token == "rightNeighbour") {
				element->rightNeighbourName = NEXT_TOKEN;
			}

			else if (token == "leftNeighbour") {
				element->leftNeighbourName = NEXT_TOKEN;
			}
			else if (token == "topNeighbour" || token == "upNeighbour") {
				element->upNeighbourName = NEXT_TOKEN;
			}
			else if (token == "bottomNeighbour" || token == "downNeighbour") {
				element->downNeighbourName = NEXT_TOKEN;
			}
			else if (token == "AddTo") {
				if (tokens.Size() > 1) {
					String parentName = NEXT_TOKEN;
					UIElement * e = NULL;
					if (parentName == "root")
						e = root;
					else
						e = root->GetElementByName(parentName);
					if (e == NULL) {
						std::cout << "\nUndefined parent element " << tokens[1] << " for element " << element->name << "! Make sure you define it before you add children to it! o-o";
						for (int c = 0; c < parentName.Length(); ++c)
							std::cout << "\nc " << std::setw(2) << " (int)" << (int)parentName.c_str()[c] << " (char)" << parentName.c_str()[c];
						break;
					}
					std::cout << "\nAdding element " << element->name << " as child to " << e->name;
					int childrenPre = e->children.Size();
					e->AddChild(nullptr, element);
					int children = e->children.Size();
					assert(children > childrenPre);
				}
				else {
					assert(element && "Element NULL! No element has been defined or it was already added! o-o");
					if (element == NULL)
						break;
					root->AddChild(nullptr, element);
				}
				element = NULL;
			}
			else {
				//	assert(false && "Unknown token in UserInterface::Load(fromFile)");
				std::cout << "\nUnknown token in UserInterface::Load(fromFile): " << token;

			}
			// By default proceed with next row straight away
			t = tokens.Size();
		}
	}
	ADD_PREVIOUS_TO_UI_IF_NEEDED

	parseStart.Stop();
	int ms = parseStart.GetMs();
	if (ms > 50) // If more than 10 ms?
		LogMain("Parsing UI at " + filePath+ " took more than 50ms: " + String(ms) + " ms!", WARNING);
	else if (ms > 5)
		LogMain("Parsing UI at "+ filePath +" took " + String(ms) + " ms", INFO);

	return root;
};

List<String> UIParser::ParseTokens(String fromLine)
{
	LogMain("Parsing tokens from line: " + fromLine, DEBUG);
	List<String> tokens;
	for (int l = 0; l < fromLine.Length(); ++l) {
		cChar = fromLine.At(l);
		//		std::cout<<"\nChar at "<<l<<": int("<<(int)cChar<<") char: "<<cChar;
		switch (cChar)
		{
			// If not in a current stack, save as a separate word.
		case ' ':
		case '\t':
		case '\n':
		case '\r':
		case '\f':
			if (!stack.Size()) {
				// Add it.
				String t;
				for (int j = lastEvaluatedIndex; j < l; j++) {
					t += fromLine.At(j);
				}
				t.RemoveInitialWhitespaces();
				if (t.Length())
					tokens.Add(t);
				lastEvaluatedIndex = l;
			}
			break;
		case '(':
			stack.Add(cChar);
			break;
		case ')':
			last = stack.Last();
			assert(last == '(');
			stack.RemoveIndex(stack.Size() - 1);
			break;
		default:
			;
		}
	}

	// Add final word as needed.
	String tok;
	for (int j = lastEvaluatedIndex; j < fromLine.Length(); j++) {
		tok += fromLine.At(j);
	}
	tok.RemoveInitialWhitespaces();
	if (tok.Length())
		tokens.Add(tok);

	return tokens;
}


void UIParser::EnsureNextToken(const List<String> fromTokens) {
	assert(fromTokens.Size() >= 2);
}
String UIParser::NextToken(const List<String> fromTokens) {
	return fromTokens[1];
}

void UIParser::SetDefaults(UIElement * element) {
	element->alignment = defaultAlignment; 
	element->textureSource = defaultTexture; 
	element->scalable = defaultScalability; 
	element->text.color = defaultTextColor; 
	element->sizeRatioY = defaultSizeRatioY; 
	element->sizeRatioX = defaultSizeRatioX; 
	element->padding = defaultPadding; 
	element->textSizeRatio = defaultTextSize; 
	element->onTrigger = defaultOnTrigger; 
	element->fontSource = TextFont::defaultFontSource; 
	element->visible = defaultVisibility; 
	element->divider = defaultDivider; 
	element->textAlignment = defaultTextAlignment; 
	element->exitable = defaultExitability; 
	element->topBorderTextureSource = defaultTopBorder; 
	element->rightBorderTextureSource = defaultRightBorder; 
	element->topRightCornerTextureSource = defaultTopRightCorner; 
}

void UIParser::AddPreviousToUIIfNeeded() {
	// Skip root if it was custom-typed.
	if (element == root)
		return;
	if (element && element != root) {
		bool addedOK = root->AddToParent(nullptr, defaultParent, element); 
		if (!addedOK)
			delete element; 
		else
			element->CreateChildren(nullptr); 
	}
	element = NULL; 
}




