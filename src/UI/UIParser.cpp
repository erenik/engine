/// Emil Hedemalm
/// 2020-08-12
/// Parser for UI, elements and their tags.

#include "UIParser.h"
#include "Graphics/Fonts/TextFont.h"
#include "File/LogFile.h"
#include <fstream>
#include "String/StringUtil.h"
#include "Globals.h"
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
#include "UI/UIFileBrowser.h"
#include "UI/UILabel.h"
#include <iomanip>

// Parsing stuff

#define ADD_PREVIOUS_TO_UI_IF_NEEDED AddPreviousToUIIfNeeded(graphicsState);

UIParser::UIParser()
	: defaultAlignment(NULL_ALIGNMENT)
	, defaultTexture ("")
	, defaultParent ("root")
	, defaultRootFolder ("")
	, defaultTopBorder ("")
	, defaultBottomBorder("")
	, defaultRightBorder ("")
	, defaultTopRightCorner ( "")
	, defaultScalability ( true)
	, defaultVisibility ( true)
	, defaultExitability ( true)
	, defaultTextColor ( nullptr )
	, defaultSizeRatioY ( 1.0f)
	, defaultSizeRatioX ( 1.0f)
	, defaultPadding ( 0.0f)
	, defaultTextSize ( 1.0f)
	, defaultOnTrigger ( "")
	, defaultDivider ( Vector2f(0.5f, 0.5f))
	, defaultTextAlignment ( LEFT)
	, defaultFontSource("")
	, defaultFontShader("")
	, lastEvaluatedIndex ( 0)
	, element(nullptr)
	, root(nullptr)
{
}

UIParser::~UIParser() {
	defaultTextColor = nullptr;
}


/// Loads from target file, using given root as root-element in the UI-hierarchy.
UIElement* UIParser::LoadFromFile(GraphicsState* graphicsState, String filePath, UserInterface * ui){
		
	// Set defaults from global (possibly game-specific) vars to initialize the parser with font, fontShader data.
	defaultFontShader = String(TextFont::defaultFontShader);
	defaultFontSource = String(TextFont::defaultFontSource);
	this->defaultForceUpperCase = UIText::defaultForceUpperCase;

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

		if (line.StartsWith("//"))
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
			ParseDefaults(graphicsState, tokens);
			if (token == "root") {
				if (tokens.Size() > 1) {
					delete root;
					String typeName = tokens[1];
					if (typeName == "ToggleButton") {
						root = new UIToggleButton();
					}
					else if (typeName == "Button") {
						root = new UIButton("root");
					}
					else if (typeName == "CompositeButton") {
						root = new UICompositeButton("root");
					}
					else if (typeName == "List") {
						root = new UIList("root");
					}
					else
						assert(false && "Unsupported root-type");
					root->SetRootDefaults(ui);
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
				AddPreviousToUIIfNeeded(graphicsState);
				element = new UIButton();
				SetDefaults(element);
				if (tokens.Size() > 1) {
					element->name = firstQuote;
					/// Set the elements text and message default to it's name too, yo.
					element->activationMessage = element->name;
					element->SetText(element->name);
				}
			}
			else if (token == "Label") {
				AddPreviousToUIIfNeeded(graphicsState);
				element = new UILabel();
				SetDefaults(element);
				if (tokens.Size() > 1) {
					element->name = tokens[1];
					element->SetText(firstQuote);
				}
			}
			else if (token == "TextField" || token == "Input")
			{
				AddPreviousToUIIfNeeded(graphicsState);
				element = new UITextField();
				SetDefaults(element);
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
				AddPreviousToUIIfNeeded(graphicsState);
				String firstToken = tokens[1];
				UICheckbox * checkBox = new UICheckbox(firstToken);
				SetDefaults(checkBox);
				checkBox->SetText(firstQuote, false);
				checkBox->CreateChildren();
				if (line.Contains("Checked"))
					checkBox->SetToggled(true);
				element = checkBox;
			}
			else if (token == "DropDownMenu" || token == "DropDownList")
			{
				ADD_PREVIOUS_TO_UI_IF_NEEDED;
				UIDropDownMenu * ddm = new UIDropDownMenu(firstQuote);
				element = ddm;
				SET_DEFAULTS;
				ddm->CreateChildren(nullptr);
			}
			else if (token == "FileBrowser") {
				AddPreviousToUIIfNeeded(graphicsState);
				UIFileBrowser* fileBrowser = new UIFileBrowser(firstQuote, "UnassignedAction", "");
				element = fileBrowser;
				SetDefaults(element);
				fileBrowser->CreateChildren(nullptr);
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
				SetDefaults(ti);
				element = ti;
				element->SetText(firstQuote);
				ti->CreateChildren(nullptr);
			}
			else if (token == "StringInput")
			{
				ADD_PREVIOUS_TO_UI_IF_NEEDED;
				String firstToken = tokens[1];
				UIStringInput * si = new UIStringInput(firstToken, "Set" + firstToken);
				SetDefaults(si);
				element = si;
				element->SetText(firstQuote);
			}
			else if (token == "FileInput") {
				AddPreviousToUIIfNeeded(graphicsState);
				String firstToken = tokens[1];
				UIFileInput * fi = new UIFileInput(firstToken, "Set" + firstToken);
				SetDefaults(fi);
				element = fi;
				fi->SetText(firstQuote);
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
				SetDefaults(ii);
				ii->SetText(firstQuote);
				element = ii;
			}
			else if (token == "IntegerLabel") // Creates an Integer-display which is not interactable via GUI, just for display.
			{
				ADD_PREVIOUS_TO_UI_IF_NEEDED;
				String firstToken = tokens[1];
				UIIntegerInput * ii = new UIIntegerInput(firstToken, "Set" + firstToken);
				SetDefaults(ii);
				element = ii;
				ii->SetText(firstQuote);
				element->interaction.hoverable = element->interaction.activateable = false;
				ii->guiInputDisabled = true;
			}
			else if (token == "StringLabel") // Creates an String-display which is not interactable via GUI, just for display.
			{
				ADD_PREVIOUS_TO_UI_IF_NEEDED;
				String firstToken = tokens[1];
				UIStringInput * si = new UIStringInput(firstToken, "Set" + firstToken);
				SetDefaults(si);
				element = si;
				element->SetText(firstQuote);
				element->interaction.hoverable = element->interaction.activateable = false;
				si->guiInputDisabled = true;
			}
			else if (token == "FloatInput") {
				ADD_PREVIOUS_TO_UI_IF_NEEDED;
				String firstToken = tokens[1];
				UIFloatInput * fi = new UIFloatInput(firstToken, "Set" + firstToken);
				SetDefaults(fi);
				element = fi;
				element->SetText(firstQuote);
			}
			else if (token == "FloatLabel") {
				AddPreviousToUIIfNeeded(graphicsState);
				String firstToken = tokens[1];
				UIFloatInput * fi = new UIFloatInput(firstToken, "Set" + firstToken);
				SetDefaults(fi);
				element = fi;
				element->SetText(firstQuote);
				element->interaction.hoverable = element->interaction.activateable = false;
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
				AddPreviousToUIIfNeeded(graphicsState);
				String action = "Set" + secondQuote;
				if (firstQuote.IsNumber()) {
					UIVectorInput * vi = new UIVectorInput(firstQuote.ParseInt(), secondQuote, action);
					SetDefaults(vi);
					element = vi;
				}
				else {
					LogGraphics("Unable to parser VectorInput, first argument should be amount of inputs to be editable!", FATAL);
				}
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
				SetDefaults(rb);
				element = rb;
				displayText.Remove('\"', true);
				rb->SetText(displayText);
			}
			else if (token == "Image")
			{
				AddPreviousToUIIfNeeded(graphicsState);
				UIImage * image = new UIImage(firstQuote, secondQuote);
				element = image;
				SetDefaults(image);
				if (secondQuote.Length())
					image->visuals.textureSource = secondQuote;
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
				element->interaction.navigateUIOnPush = true;
			}
			else if (token == "disabled")
			{
				element->Disable();
			}
			else if (token == "forceUpperCase") {
				EnsureNextToken(tokens);
				element->text.forceUpperCase = tokens[1].ParseBool();
			}
			else if (token == "noBorders") {
				if (element != nullptr)
					element->rightBorderTextureSource =
					element->topBorderTextureSource =
					element->bottomBorderTextureSource =
					element->topRightCornerTextureSource = "";
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
			else if (token == "SelectionsTextColor") {
				UIRadioButtons * radioButtons = (UIRadioButtons*)element;
				radioButtons->SetSelectionsTextColor(Color::ColorByHexName(firstQuote));
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
			else if (token == "FileFilter") {
				if (element->type == UIType::FILE_BROWSER) {
					UIFileBrowser * fileBrowser = (UIFileBrowser *)element;
					fileBrowser->SetFileFilter(firstQuote);
				}
				else if (element->type == UIType::FILE_INPUT) {
					UIFileInput * fi = (UIFileInput*)element;
					fi->SetFileFilter(firstQuote);
				}
				else
					LogGraphics("Unable to set file filter in anything but a File Browser", INFO);
			}
			else if (token == "displayText")
			{
				EnsureNextToken(tokens);
				element->SetText(firstQuote);
			}
			else if (token == "UseToggleTexts") {
				EnsureNextToken(tokens);
				UICheckbox * checkbox = (UICheckbox * )element;
				checkbox->SetToggleTexts(tokens[1], tokens[2]);
			}
			else if (token == "Padding") {
				EnsureNextToken(tokens);
				element->layout.padding = NEXT_TOKEN.ParseFloat();
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
					element->interaction.onExit = NEXT_TOKEN;
			}
			else if (token == "exitable") {
				ENSURE_NEXT_TOKEN
					element->interaction.exitable = (NEXT_TOKEN).ParseBool();
			}
			else if (token == "hoverable")
			{
				ENSURE_NEXT_TOKEN;
				element->interaction.hoverable = NEXT_TOKEN.ParseBool();
			}
			else if (token == "activatable") {
				EnsureNextToken(tokens);
				element->interaction.activateable = NextToken(tokens).ParseBool();
			}
			else if (token == "deleteOnPop") {
				EnsureNextToken(tokens);
				element->deleteOnPop = NextToken(tokens).ParseBool();
			}
			else if (token == "highlightOnHover")
			{
				EnsureNextToken(tokens);
				element->visuals.highlightOnHover = NextToken(tokens).ParseBool();
			}
			else if (token == "highlightOnActive") {
				EnsureNextToken(tokens);
				element->visuals.highlightOnActive = NextToken(tokens).ParseBool();
			}
			else if (token == "LabelText")
			{
				ENSURE_NEXT_TOKEN
					element->labelText = firstQuote;
			}
			else if (token == "visible" || token == "visibility")
			{
				ENSURE_NEXT_TOKEN
					element->interaction.visible = (NEXT_TOKEN).ParseBool();
			}
			else if (token == "font") {
				EnsureNextToken(tokens);
				element->text.fontDetails.source = firstQuote;
			}
			else if (token == "fontShader") {
				EnsureNextToken(tokens);
				element->text.fontDetails.shader = tokens[1];
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
				case UIType::VECTOR_INPUT:
				{
					UIVectorInput * vi = (UIVectorInput*)element;
					vi->maxDecimals = NextToken(tokens).ParseInt();
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
					element->visuals.textureSource = String();
				else
					element->visuals.textureSource = defaultRootFolder + param;
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
			else if (token == "OnHoverTextColor") {
				element->SetOnHoverTextColor(Color::ColorByHexName(line - "OnHoverTextColor"));
			}
			else if (token == "textColor")
			{
				String nextToken = NEXT_TOKEN;
				// Hex detected!
				if (line.Contains("0x") || line.Contains("#"))
				{
					element->SetTextColors(Color::ColorByHexName(nextToken));
				}
				else
				{
					Color color;
					switch (tokens.Size() - 1)
					{
					case 1: // Assume it's alpha and keep the other colors as usual
						color.SetAlpha(NEXT_TOKEN.ParseFloat());
						break;
					case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11: case 12:
						color.SetAlpha(tokens[4].ParseFloat());
					case 3: // Assume it's RGB
						color.SetRGB(tokens[1].ParseFloat(), tokens[2].ParseFloat(), tokens[3].ParseFloat());
						break;
					case 2: case 0:
						assert(false && "Irregular amount of tokens following \"textColor\"; 1 for alpha, 3 for RGB and 4 for RGBA.");
						break;
					}
					element->SetTextColors(color);
				}
			}
			else if (token == "textSizeRatio" || token == "textSize") {
				ENSURE_NEXT_TOKEN
					element->text.sizeRatio = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "textAlignment")
			{
				value.SetComparisonMode(String::NOT_CASE_SENSITIVE);
				if (value == "Center")
				{
					element->text.alignment = CENTER;
				}
				else if (value == "Right") {
					element->text.alignment = RIGHT;
				}
				else if (value == "Left") {
					element->text.alignment = LEFT;
				}
			}
			else if (token == "dividerX")
			{
				EnsureNextToken(tokens);
				element->divider.x = NextToken(tokens).ParseFloat();
			}
			else if (token == "textPadding") {
				EnsureNextToken(tokens);
				element->text.paddingPixels = NextToken(tokens).ParseInt();
			}
			else if (token == "origin") {
				ENSURE_NEXT_TOKEN
					element->origin = NEXT_TOKEN.ParseInt();
			}
			else if (token == "scalable") {
				EnsureNextToken(tokens);
				element->scalable = NEXT_TOKEN.ParseBool();
			}
			else if (token == "formatX") {
				EnsureNextToken(tokens);
				element->formatX = NEXT_TOKEN.ParseBool();
			}
			else if (token == "lineSizeRatio")
			{
				EnsureNextToken(tokens);
				element->text.lineSizeRatio = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "sizeRatioX") {
				ENSURE_NEXT_TOKEN
					element->layout.sizeRatioX = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "sizeRatioY") {
				ENSURE_NEXT_TOKEN
					element->layout.sizeRatioY = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "sizeRatio") {
				ENSURE_NEXT_TOKEN
					element->layout.sizeRatioX = element->layout.sizeRatioY = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "sizeRatioXY") {
				if (tokens.Size() < 3)
					continue;
				element->layout.sizeRatioX = tokens[1].ParseFloat();
				element->layout.sizeRatioY = tokens[2].ParseFloat();
			}
			else if (token == "alignment") {
				EnsureNextToken(tokens);
				element->layout.alignment = GetAlignment(NEXT_TOKEN);
			}
			else if (token == "retainAspectRatio") {
				EnsureNextToken(tokens);
				element->visuals.retainAspectRatioOfTexture = NextToken(tokens).ParseBool();
			}
			else if (token == "maxWidth") {
				EnsureNextToken(tokens);
				element->layout.maxWidth = tokens[1].ParseInt();
			}
			else if (token == "alignmentX") {
				EnsureNextToken(tokens);
				element->layout.alignmentX = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "bottomBorder") {
				EnsureNextToken(tokens);
				element->bottomBorderTextureSource = line - "bottomBorder";
				element->bottomBorderTextureSource.RemoveSurroundingWhitespaces();
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
				if (element->rightBorderTextureSource == "null")
					element->rightBorderTextureSource = "";
			}
			else if (token == "InputTexture") {
			UIInput * input = (UIInput*)element;
			input->SetInputTexture(firstQuote);
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
					element->layout.alignmentY = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "alignmentXY" || token == "alignment") {
				if (tokens.Size() < 3)
					continue;
				element->layout.alignmentX = tokens[1].ParseFloat();
				element->layout.alignmentY = tokens[2].ParseFloat();
				int b = element->layout.alignmentY + 3;
			}
			else if (token == "rightNeighbour") {
				element->interaction.rightNeighbourName = NEXT_TOKEN;
			}

			else if (token == "leftNeighbour") {
				element->interaction.leftNeighbourName = NEXT_TOKEN;
			}
			else if (token == "topNeighbour" || token == "upNeighbour") {
				element->interaction.upNeighbourName = NEXT_TOKEN;
			}
			else if (token == "bottomNeighbour" || token == "downNeighbour") {
				element->interaction.downNeighbourName = NEXT_TOKEN;
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
	element->layout.alignment = defaultAlignment;
	element->visuals.textureSource = defaultTexture;
	element->scalable = defaultScalability;
	if (defaultTextColor)
		element->SetTextColors(*defaultTextColor);
	element->layout.sizeRatioY = defaultSizeRatioY; 
	element->layout.sizeRatioX = defaultSizeRatioX; 
	element->layout.padding = defaultPadding; 
	element->text.paddingPixels = defaultTextPadding;
	element->text.sizeRatio = defaultTextSize; 
	element->onTrigger = defaultOnTrigger; 
	element->text.fontDetails.source = defaultFontSource;
	element->text.fontDetails.shader = defaultFontShader;
	element->interaction.visible = defaultVisibility; 
	element->divider = defaultDivider; 
	element->text.alignment = defaultTextAlignment; 
	element->SetForceUpperCase(defaultForceUpperCase);
	element->interaction.exitable = defaultExitability; 
	element->topBorderTextureSource = defaultTopBorder;
	element->bottomBorderTextureSource = defaultBottomBorder;
	element->rightBorderTextureSource = defaultRightBorder; 
	element->topRightCornerTextureSource = defaultTopRightCorner; 
}

void UIParser::AddPreviousToUIIfNeeded(GraphicsState* graphicsState) {
	// Skip root if it was custom-typed.
	if (element == root)
		return;
	if (element && element != root) {
		bool addedOK = root->AddToParent(graphicsState, defaultParent, element);
		if (!addedOK)
			delete element; 
		else
			element->CreateChildren(graphicsState); 
	}
	element = NULL; 
}


// Checks for defaultSize, defaultTexture, etc.
void UIParser::ParseDefaults(GraphicsState * graphicsState, List<String> tokens) {
	String token = tokens[0];
	String value;
	if (tokens.Size() > 1)
		value = tokens[1];
	String token2 = value;

	if (token == "defaultAlignment") {
		EnsureNextToken(tokens);
		EnsureNextToken(tokens);
		defaultAlignment = GetAlignment(NEXT_TOKEN);
	}
		else if (token == "defaultTextAlignment")
	{
	EnsureNextToken(tokens);
		defaultTextAlignment = GetAlignment(value);
	}
	else if (token == "defaultTexture") {
		EnsureNextToken(tokens);
			String param = tokens[1];
		param.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		if (param == "NULL")
			defaultTexture = String();
		else
			defaultTexture = param;
	}
	else if (token == "defaultFont") {
		defaultFontSource = tokens[1];
	}
	else if (token == "defaultFontShader") {
		defaultFontShader = tokens[1];
	}
	else if (token == "defaultTopBorder") {
		EnsureNextToken(tokens);
		defaultTopBorder = (line - "defaultTopBorder");
		defaultTopBorder.RemoveSurroundingWhitespaces();
		if (defaultTopBorder == "null")
			defaultTopBorder = "";
	}
	else if (token == "defaultRightBorder") {
		EnsureNextToken(tokens);
		defaultRightBorder = (line - "defaultRightBorder");
		defaultRightBorder.RemoveSurroundingWhitespaces();
		if (defaultRightBorder == "null")
			defaultRightBorder = "";
	}
	else if (token == "defaultTopRightCorner") {
		EnsureNextToken(tokens);
		defaultTopRightCorner = (line - "defaultTopRightCorner");
		defaultTopRightCorner.RemoveSurroundingWhitespaces();
		if (defaultTopRightCorner == "null")
			defaultTopRightCorner = "";
	}
	else if (token == "defaultOnTrigger") {
		EnsureNextToken(tokens);
			defaultOnTrigger = NEXT_TOKEN;
	}
	else if (token == "defaultParent" ||
		token == "parent")
	{
		AddPreviousToUIIfNeeded(graphicsState);
		EnsureNextToken(tokens);
		defaultParent = NEXT_TOKEN;
	}
		else if (token == "defaultScalability") {
		EnsureNextToken(tokens);
			defaultScalability = NEXT_TOKEN.ParseBool();
	}
	else if (token == "defaultExitability")
	{
		defaultExitability = value.ParseBool();
	}
	else if (token == "defaultForceUpperCase") {
		EnsureNextToken(tokens);
		defaultForceUpperCase = value.ParseBool();
	}
	else if (token == "defaultDividerX")
	{
		EnsureNextToken(tokens);
		defaultDivider.x = NextToken(tokens).ParseFloat();
	}
	else if (token == "defaultVisibility") {
		EnsureNextToken(tokens);
			defaultVisibility = NEXT_TOKEN.ParseBool();
	}
	else if (token == "defaultTextPadding") {
		EnsureNextToken(tokens);
		defaultTextPadding = NextToken(tokens).ParseInt();
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
		EnsureNextToken(tokens);
			defaultSizeRatioY = NEXT_TOKEN.ParseFloat();
	}
	else if (token == "defaultSizeRatioX") {
		EnsureNextToken(tokens);
			defaultSizeRatioX = NEXT_TOKEN.ParseFloat();
	}
	else if (token == "defaultPadding") {
		EnsureNextToken(tokens);
			defaultPadding = NEXT_TOKEN.ParseFloat();
	}
	else if (token == "defaultTextSize") {
		EnsureNextToken(tokens);
			defaultTextSize = NEXT_TOKEN.ParseFloat();
	}
	else if (token == "defaultTextColor")
	{
		SAFE_DELETE(defaultTextColor);
		if (line.Contains("null"))
			return;

		defaultTextColor = new Color();
		Color& newDefaultTextColor = *defaultTextColor;
		// Hex detected!
		if (line.Contains("0x"))
		{
			newDefaultTextColor = Color::ColorByHexName(NEXT_TOKEN);
		}
		else if (line.Contains("#")) {
			newDefaultTextColor = Color::ColorByHexName(NEXT_TOKEN);
		}
		else
		{
			switch (tokens.Size() - 1)
			{
			case 1: // Assume it's alpha and keep the other colors as usual
				newDefaultTextColor.w = NEXT_TOKEN.ParseFloat();
				break;
			case 4:
				newDefaultTextColor.w = tokens[4].ParseFloat();
			case 3: // Assume it's RGB
				newDefaultTextColor.SetRGB(tokens[1].ParseFloat(), tokens[2].ParseFloat(), tokens[3].ParseFloat());
				break;
			case 2: case 0:
				assert(false && "Irregular amount of tokens following \"defaultTextColor\"; 1 for alpha, 3 for RGB and 4 for RGBA.");
				break;
			}
		}
		LogMain("Default text color updated to: " + ColorString(defaultTextColor), INFO);
	}
	else if (token == "defaultRootFolder") {
		EnsureNextToken(tokens);
			defaultRootFolder = NEXT_TOKEN + "/";
		if (NEXT_TOKEN == "NULL")
			defaultRootFolder = "";
	}
}
// Checks for Button, Checkbox, List, etc.
void UIParser::ParseNewUIElements() {

}

