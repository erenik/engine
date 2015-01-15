/// Emil Hedemalm
/// 2014-04-09
/// OpenCV Pipeline for handling input, filters, calculation-filters (working with points/blobs) and output.

#include "CVPipeline.h"
#include "Timer/Timer.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMUI.h"
#include <fstream>
#include <iomanip>

#include "CV/RenderFilters/CVRenderFilters.h"

#include "UI/UserInterface.h"
#include "UI/UITypes.h"
#include "UI/UIList.h"
#include "UI/UIInputs.h"
#include "UI/UIButtons.h"
#include "UI/UIFileBrowser.h"

#include "Window/WindowManager.h"

#include "Message/MessageTypes.h"
#include "Message/Message.h"
#include "Message/MathMessage.h"
#include "Message/FileEvent.h"
#include "Message/MessageManager.h"

#undef ERROR

CVPipeline::CVPipeline()
{
	outputProjectionRelativeSizeInInput = Vector3f(1,1,1);
	editorWindow = NULL;
	filterSelectionFilter = NULL;
	currentEditFilter = NULL;
}
CVPipeline::~CVPipeline()
{
	filters.ClearAndDelete();
}

/// Opens up an editor-window for this CVPipeline, assuming the existance of PipelineEditor GUI file. (gui/PipelineEditor.gui)
Window * CVPipeline::OpenEditorWindow()
{
	// Look for an existing lighting-editor window.
	// Create it if not existing.
	if (!editorWindow)
	{
		editorWindow = WindowMan.NewWindow("CVPipelineEditor", "CV Pipeline editor");
		UserInterface * ui = editorWindow->CreateUI();
		ui->Load("gui/CVPipelineEditor.gui");
		editorWindow->SetRequestedSize(Vector2i(550, 550));
		editorWindow->DisableAllRenders();
		editorWindow->renderUI = true;
		editorWindow->CreateGlobalUI();
		editorWindow->Create();
	}
	
	// Show it.
	editorWindow->Show();
	// Bring it to the top if needed.
	editorWindow->BringToTop();

	
	// Set default values as specified in this lighting.
	UserInterface * ui = editorWindow->ui;

//	Graphics.QueueMessage(new GMSetUIv3f("GlobalAmbient", GMUI::VECTOR_INPUT, GetAmbient(), ui));

//	UpdateLightList(window);

	// Update it straight away.
	OnPipelineUpdated();

	return editorWindow;
}

/// Call every time after changing the pipeline. Updates the UI in the dedicated winodw.
void CVPipeline::OnPipelineUpdated()
{
	if (!editorWindow)
		return;

	/// Fetch previous position in the list.
	float prevListPos = 0.0f;
	UserInterface * ui = ActiveUI();
	if (ui)
	{
		UIElement * element = ui->GetElementByName("Filters");
		if (element && element->type == UIType::LIST)
		{
			UIList * list = (UIList*)element;
			prevListPos = list->GetScrollPosition();
		}
	}


	/// Populate the list.
	Graphics.QueueMessage(new GMClearUI("Filters", editorWindow->ui));
	/// Add filters
	for (int i = 0; i < filters.Size(); ++i)
	{
		CVFilter * filter = filters[i];
		String filterName = filter->name;
		String indexString = String::ToString(i);
		// Column-list to handle the buttons for editing this filter.
		UIColumnList * filterCList = new UIColumnList(filter->name+"ColumnList");
		filterCList->sizeRatioY = 0.1f;

		UIButton * button = new UIButton("EditFilter"+filter->name);
		button->text = filterName;
		button->sizeRatioX = 0.4f;
		button->activationMessage = "EditFilter:"+String::ToString(i);
		filterCList->AddChild(button);

		// For temporarily disabling a specific filter (don't wanna delete 'em all the time! o.o)
		UIButton * toggleButton = new UIButton("ToggleFilterButton"+indexString);
		toggleButton->sizeRatioX = 0.15f;
		if (filter->enabled)
			toggleButton->text = "Disable";
		else 
			toggleButton->text = "Enable";
		toggleButton->activationMessage = "ToggleFilter:"+indexString;
		filterCList->AddChild(toggleButton);

		// For changing order in the pipeline.
		UIButton * moveUpButton = new UIButton(filter->name+"MoveUp");
		moveUpButton->sizeRatioX = 0.15f;
		moveUpButton->text = "Move up";
		moveUpButton->activationMessage = "MoveFilterUp:"+String::ToString(i);
		filterCList->AddChild(moveUpButton);
		UIButton * moveDownButton = new UIButton(filter->name+"MoveDown");
		moveDownButton->sizeRatioX = 0.15f;
		moveDownButton->text = "Move down";
		moveDownButton->activationMessage = "MoveFilterDown:"+String::ToString(i);
		filterCList->AddChild(moveDownButton);

		// Add a button to delete this filter too.
		UIButton * deleteButton = new UIButton(filter->name+"DeleteButton");
		deleteButton->text = "Delete"; 
		deleteButton->sizeRatioX = 0.15f;
		deleteButton->activationMessage = "DeleteFilter:"+String::ToString(i);
		filterCList->AddChild(deleteButton);
		/// Finally add the entire button/ui for this filter
		Graphics.QueueMessage(new GMAddUI(filterCList, "Filters", editorWindow->ui));
	}

	// Set list position again.
	Graphics.QueueMessage(new GMSetUIf("Filters", GMUI::SCROLL_POSITION_Y, prevListPos, editorWindow->ui));
}


void CVPipeline::OpenFilterSelectionMenu()
{
	// Populate filters-list.
	OnFilterTypesUpdated();
	Graphics.QueueMessage(new GMPushUI("FilterSelectionMenu", editorWindow->ui));
}

/// Update filters available in the filter-selection menu (for appending/inserting new filters!)
void CVPipeline::OnFilterTypesUpdated()
{
		/// Populate the list.
	Graphics.QueueMessage(new GMClearUI("FilterSelectionList", editorWindow->ui));
	/// Add new filters.
	List<String> filterList = CVFilter::filterNames;
	for (int i = 0; i < filterList.Size(); ++i)
	{
		CVFilter * filter = GetSampleFilter(i);
		if (!filter)
			continue;
		String filterName = filterList[i];
		if (!filterName.Length())
			continue;
		if (filter->Type() != filterSelectionFilter)
			continue;
		UIButton * button = new UIButton("AddFilter:"+filterName);
		button->text = filterName;
		button->sizeRatioY = 0.05f;
		button->activationMessage = "AddFilter:"+filterList[i]+"&&PopUI(FilterSelectionMenu)";
		Graphics.QueueMessage(new GMAddUI(button, "FilterSelectionList", editorWindow->ui));
	}
}


/// Loads target pipeline config, sends a message that it has been loaded upon completion.
void CVPipeline::LoadPipelineConfig(String fromFile)
{
	std::fstream file;
	file.open(fromFile.c_str(), std::ios_base::in | std::ios_base::binary);
	if (file.is_open())
	{
		bool success = ReadFrom(file);
		if (!success){
			file.close();
			OnPipelineUpdated();
	//		Log ("Error loading file: "+pipeline->GetLastError());
			return;
		}
		else 
		;//	Log("Pipeline configuration loaded from file: "+fromFile);
	}
	else {
	;//	Log("Unable to open file: "+fromFile);
	}
	file.close();
	// Update ui
	OnPipelineUpdated();
	// Notify stuff
	MesMan.QueueMessages("PipelineLoadedSuccessfully");
}

/// Updates the text in the Enable/Disable button
void CVPipeline::OnFilterEnabledUpdated(CVFilter * filter)
{
	/// Update the enabled-text of the filter.
	int index = filters.GetIndexOf(filter);
	String indexString = String(index);
	Graphics.QueueMessage(new GMSetUIs("ToggleFilterButton"+indexString, GMUI::TEXT, filter->enabled? "Disable" : "Enable", editorWindow->ui));		
}

/// Selects the filter, pushing it's ui onto the ui dedicated to handling filter edits.
void CVPipeline::SelectFilter(int byIndex)
{
	CVFilter * filter = filters[byIndex];

	// Use the same ui as the pipeline is using
	UserInterface * ui = editorWindow->ui;

	/// Update the contents of the filter-editor.
	filter->UpdateEditUI(ui);

	/// And make visible the filter-editor.
	Graphics.QueueMessage(new GMPushUI("FilterEditor", ui));

	// Set the filter as current for future edits.
	currentEditFilter = filter;
}

/// Passes on the message to all filters.
void CVPipeline::ProcessMessage(Message * message)
{
	for (int i = 0; i < filters.Size(); ++i)
	{
		CVFilter * filter = filters[i];
		filter->ProcessMessage(message);
	}
	String msg = message->msg;
	switch(message->type)
	{
		case MessageType::BOOL_MESSAGE:
		{
			BoolMessage * bm = (BoolMessage*) message;
			if (msg.Contains("SetBool:"))
			{
				String settingName = msg.Tokenize(":")[1];
				CVFilterSetting * setting = currentEditFilter->GetSetting(settingName);
				assert(setting && "Setting not found. You missed something while coding");
				if (!setting)
					return;
				setting->SetBool(bm->value);
				/// Signal the user of the pipeline that something changed.
				MesMan.QueueMessages("CVFilterSettingUpdated");
			}
			break;
		}
		case MessageType::VECTOR_MESSAGE:
		{
			VectorMessage * vm = (VectorMessage*) message;
			if (msg.Contains("SetVector:"))
			{
				String settingName = msg.Tokenize(":")[1];
				CVFilterSetting * setting = currentEditFilter->GetSetting(settingName);
				assert(setting && "Setting not found. You missed something while coding");
				if (!setting)
					return;
				switch(setting->type)
				{
					case CVSettingType::VECTOR_2I:
						setting->SetVec2i(vm->vec2i);
						break;
					case CVSettingType::VECTOR_2F:
						setting->SetVec2f(vm->vec2f);
						break;
					case CVSettingType::VECTOR_3F:
						setting->SetVec3f(vm->vec3f);
						break;
					case CVSettingType::VECTOR_4F:
						setting->SetVec4f(vm->vec4f);
						break;
					default:
						assert(false);
				}
				/// Signal the user of the pipeline that something changed.
				MesMan.QueueMessages("CVFilterSettingUpdated");
			}
			break;	
		}
		case MessageType::SET_STRING:
		{
			SetStringMessage * ssm = (SetStringMessage*) message;
			if (msg.Contains("SetString:"))
			{
				String settingName = msg.Tokenize(":")[1];
				CVFilterSetting * setting = currentEditFilter->GetSetting(settingName);
				assert(setting && "Setting not found. You missed something while coding");
				if (!setting)
					return;
				setting->SetString(ssm->value);
				/// Signal the user of the pipeline that something changed.
				MesMan.QueueMessages("CVFilterSettingUpdated");
			}	
			break;
		}
		case MessageType::INTEGER_MESSAGE:
		{
			IntegerMessage * im = (IntegerMessage*) message;
			// For pipeline filter setting editing
			if (msg.Contains("SetInteger:"))
			{
				String settingName = msg.Tokenize(":")[1];
				CVFilterSetting * setting = currentEditFilter->GetSetting(settingName);
				assert(setting && "Setting not found. You missed something while coding");
				if (!setting)
					return;
				setting->SetInt(im->value);
				/// Signal the user of the pipeline that something changed.
				MesMan.QueueMessages("CVFilterSettingUpdated");
			}
			break;
		}
		case MessageType::FLOAT_MESSAGE: 
		{
			FloatMessage * fm = (FloatMessage*) message;
			// For pipeline filter setting editing
			if (msg.Contains("SetFloat:"))
			{
				String settingName = msg.Tokenize(":")[1];
				CVFilterSetting * setting = currentEditFilter->GetSetting(settingName);
				assert(setting && "Setting not found. You missed something while coding");
				if (!setting)
					return;
				setting->SetFloat(fm->value);
				/// Signal the user of the pipeline that something changed.
				MesMan.QueueMessages("CVFilterSettingUpdated");
			}
			break;
		}
		case MessageType::FILE_EVENT:
		{
			FileEvent * fe = (FileEvent*) message;
			if (false){}
			else if (msg == "SavePipelineConfig")
			{
				String fileName = fe->files[0];
				std::fstream file;
				if (!fileName.Contains(PIPELINE_CONFIG_FILE_ENDING))
					fileName += PIPELINE_CONFIG_FILE_ENDING;
				file.open(fileName.c_str(), std::ios_base::out | std::ios_base::binary);
				if (file.is_open())
				{
					// Shouldn't need any mutex..
					WriteTo(file);
//					Log("Pipeline configuration saved to file: "+fileName);
				}
				else {
	//				Log("Unable to save to file: "+fileName);
				}
				file.close();
			}
			else if (msg == "LoadPipelineConfig")
			{
				// Close filter editor so no old stuff will be manipulated.
				Graphics.QueueMessage(new GMPopUI("FilterEditor", editorWindow->ui));

				String fileName = fe->files[0];
				LoadPipelineConfig(fileName);
			}
			break;	
		}
		case MessageType::STRING:
		{
			if (false)
			{
			}
				/// Edit-mode selection
			else if (msg.Contains("ActivateButtonSetting:"))
			{
				// Sent from a checkbox.. probably.
				String settingName = msg.Tokenize(":")[1];
				CVFilterSetting * setting = currentEditFilter->GetSetting(settingName);
				assert(setting && "Setting not found. You missed something while coding");
				if (!setting)
					return;
				setting->SetBool(true);
				/// Signal the user of the pipeline that something changed.
				MesMan.QueueMessages("CVFilterSettingUpdated");
			}
			else if (msg.Contains("SetBool:"))
			{
				// Sent from a checkbox.. probably.
				String settingName = msg.Tokenize(":")[1];
				CVFilterSetting * setting = currentEditFilter->GetSetting(settingName);
				assert(setting && "Setting not found. You missed something while coding");
				if (!setting)
					return;
				setting->SetBool(message->element->toggled);
				/// Signal the user of the pipeline that something changed.
				MesMan.QueueMessages("CVFilterSettingUpdated");
			}
			else if (msg == "ImageFilters")
			{
				filterSelectionFilter = CVFilterType::IMAGE_FILTER;
				OnFilterTypesUpdated();
			}
			else if (msg == "DataFilters")
			{
				filterSelectionFilter = CVFilterType::DATA_FILTER;
				OnFilterTypesUpdated();
			}
			else if (msg == "RenderFilters")
			{
				filterSelectionFilter = CVFilterType::RENDER_FILTER;
				OnFilterTypesUpdated();
			}
			/// Initial messages which bring up the dialogues for saving and loadng.
			else if (msg == "SavePipeline")
			{
				// Open file dialogue for saving pipeline configuration.
				UIFileDialogue * saveDialogue = new UIFileDialogue("Save pipeline configuration", "SavePipelineConfig", PIPELINE_CONFIG_FILE_ENDING);
				saveDialogue->CreateChildren();
				saveDialogue->SetPath("data", false);
				saveDialogue->LoadDirectory(false);
				Graphics.QueueMessage(new GMAddUI(saveDialogue, "root", editorWindow->ui));
				Graphics.QueueMessage(new GMPushUI(saveDialogue, editorWindow->ui));
			}
			else if (msg == "LoadPipeline")
			{
				// Open file dialogue for loading pipeline configuration.
				UIFileDialogue * saveDialogue = new UIFileDialogue("Load pipeline configuration", "LoadPipelineConfig", PIPELINE_CONFIG_FILE_ENDING);
				saveDialogue->CreateChildren();
				saveDialogue->SetPath("data", false);
				saveDialogue->LoadDirectory(false);
				Graphics.QueueMessage(new GMAddUI(saveDialogue, "root", editorWindow->ui));
				Graphics.QueueMessage(new GMPushUI(saveDialogue, editorWindow->ui));
			}
			else if (msg.Contains("LoadPipelineConfig("))
			{
				String fileName = msg.Tokenize("()")[1];
				std::fstream file;
				file.open(fileName.c_str(), std::ios_base::in | std::ios_base::binary);
				if (file.is_open()){
					bool success = ReadFrom(file);
					if (!success)
					{
						file.close();
						OnPipelineUpdated();
					//	Log ("Error loading file: "+pipeline->GetLastError());
						return;
					}
					else 
						;//Log("Pipeline configuration loaded from file: "+fileName);
				}
				else 
				{
				//	Log("Unable to open file: "+fileName);
				}
				file.close();
				OnPipelineUpdated();
			}
		}
	}
}





/// CVPipeline versions.
#define CV_PIPELINE_VERSION_1	1

/// Save/load functions.
void CVPipeline::WriteTo(std::fstream & file)
{
	int version = CV_PIPELINE_VERSION_1;
	file.write((char*) &version, sizeof(int));
	// Save name, even though it's not in use write now.
	name.WriteTo(file);
	// Number of filters.
	int numFilters = filters.Size();
	file.write((char*)&numFilters, sizeof(int));
	// Then write the filters' themselves, with ID/names and settings.
	for (int i = 0; i < filters.Size(); ++i)
	{
		/// Write name and ID of filter.
		CVFilter * filter = filters[i];
		int id = filter->ID();
		file.write((char*)&id, sizeof(int));
		/// Then settings
		filter->WriteTo(file);
	}
	// Done!
	return;
}

bool CVPipeline::ReadFrom(std::fstream & file)
{
	/// Clear current pipeline before reading in this!
	this->Clear();

	int version = CV_PIPELINE_VERSION_1;
	file.read((char*) &version, sizeof(int));
	assert(version == CV_PIPELINE_VERSION_1);
	// Save name, even though it's not in use write now.
	name.ReadFrom(file);
	// Number of filters.
	int numFilters = filters.Size();
	file.read((char*)&numFilters, sizeof(int));
	// Then write the filters' themselves, with ID/names and settings.
	for (int i = 0; i < numFilters; ++i)
	{
		String name;
		int id;
		file.read((char*)&id, sizeof(int));
		/// Create filter of given type.
		CVFilter * filter = CreateFilterByID(id);
		if (!filter)
		{
			errorString = "Unable to read filter in file. Unknown ID.";
			return false;
		}
		filter->ReadFrom(file);
		AppendFilter(filter);
	}
	// Done!
	totalProcessingTimes.Clear();
	return true;
}

void CVPipeline::Swap(int filterAtIndex, int withFilterAtIndex)
{
	filters.Swap(filterAtIndex, withFilterAtIndex);
}
	

void CVPipeline::InsertFilter(CVFilter * newFilter, int atIndex)
{
	filters.Insert(newFilter, atIndex);
	newFilter->OnAdd();
}
void CVPipeline::AppendFilter(CVFilter * newFilter)
{
	filters.Add(newFilter);
	newFilter->OnAdd();
}

void CVPipeline::SetInitialInput(cv::Mat & mat)
{
	mat.copyTo(initialInput);

	currentScale = Vector2f(1,1);
	currentScaleInv = currentScale;

	inputSize = Vector2i(initialInput.cols, initialInput.rows);

	initialInputSize = Vector3f(initialInput.cols, initialInput.rows, 0);
}


// Clears all filters, calling OnDelete on them so that they may do proper clean-up.
void CVPipeline::Clear()
{
	while(filters.Size())
		DeleteFilterByIndex(0);

	// Should be hidden automatically within the DeleteFilter function.
	// Hide the UI for the filter editor.
//	currentEditFilter = NULL;
///	Graphics.QueueMessage(new GMPopUI("FilterEditor", ui));
}


/// Current amount of filters.
int CVPipeline::NumFilters()
{
	return filters.Size();
}
List<CVFilter*> CVPipeline::Filters()
{
	return filters;
}

/// Deletes filter. Returns the now dead pointer (still pointing to the same address-space, for address-comparison) or NULL if it fails/invalid index.
CVFilter * CVPipeline::DeleteFilterByIndex(int index)
{
	if (index < 0 || index >= filters.Size())
		return NULL;
	CVFilter * filter = filters[index];
	filter->OnDelete();
	// Remove it from the list.
	filters.RemoveIndex(index, ListOption::RETAIN_ORDER);

	// o-o;
	if (filter == currentEditFilter)
	{
		// Hide the UI for the filter editor.
		currentEditFilter = NULL;
		Graphics.QueueMessage(new GMPopUI("FilterEditor", editorWindow->ui));
	}


	// And delete it.
	delete filter;
	return filter;
}

/** Takes input image and processes it through all filters and calc-filters. 
	Returns an index specifying the type of output generated or -1 if an error occured.
*/
int CVPipeline::Process()
{
	initialInput.copyTo(input);
	/// Copy to output too, in-case of empty pipeline.
	initialInput.copyTo(output);
	// Start timer
	Timer pipelineTimer;
	pipelineTimer.Start();
	Timer filterTimer;

	CVFilter * lastProcessedFilter = NULL;
	// To avoid unnecessary painting time.
	filterToPaint = NULL;

	returnType = CVReturnType::CV_IMAGE;
	CVFilter * previousFilter = NULL;
	for (int i = 0; i < filters.Size(); ++i)
	{
		CVFilter * filter = filters[i];
		filter->previousFilter = previousFilter;
		// Skip temporarily disabled filters.
		if (!filter->enabled)
			continue;
		// Store for next loop.
		previousFilter = filter;

		// Start processing.
		filterTimer.Start();
		try {
			returnType = filter->Process(this);
		} catch (...)
		{
			std::cout<<"\nSome error was thrown while processing filter: "<<filter->name;
		}
		filter->returnType = returnType;
		filter->processingTime = filterTimer.GetMicro();

		lastProcessedFilter = filter;
		// Update the filter to paint as necessary.
		if (!filterToPaint)
			filterToPaint = lastProcessedFilter;
		else 
		{
			switch(lastProcessedFilter->ID())
			{
				// Filters to skip.
			case CVFilterID::VIDEO_WRITER:
				break;
				// Most filters are renderable.
			default:
				filterToPaint = lastProcessedFilter;
			}
		}

		/// Copy output/Render if even if it failed (might want to render some debug-failure information)
		if (returnType == CVReturnType::CV_IMAGE)
		{
			output.copyTo(input);
		}
		/// If it failed, additionally print the error information onto the screen.
		if (returnType == -1 || returnType == CVReturnType::ERROR){
			errorString = filter->name+" error: "+filter->GetLastError();
			break;
		}
		// Always call Paint on them for now... TODO: FIX THIS AS A SETTING could impact significantly
		else {
			/*
			filterTimer.Start();
			filter->Paint(this);
			filter->renderTime = filterTimer.GetMicro();
			*/
		}
		// If ok, send status info.
		if (filter == this->currentEditFilter && filter->status.Length())
			Graphics.QueueMessage(new GMSetUIs(filter->name+"Status", GMUI::TEXT, filter->status, editorWindow->ui));
	}
	// Stop timer before painting!	
	pipelineTimer.Stop();
	pipelineTimeConsumption = pipelineTimer.GetMicro();

	// Re-create array of test values if not the required amount.
	if (totalProcessingTimes.Size() != filters.Size() + 2)
	{
		totalProcessingTimes.Clear();
		for (int i = 0; i < filters.Size() + 2; ++i)
		{
			int64 newInt = 0;
			totalProcessingTimes.Add(newInt);
		}
		// Reset iterations each time we have to re-create this array.
		iterations = 0;
	}
	// Record test-values.
	totalFilterProcessingTimeThisFrame = 0;
	for (int i = 0; i < filters.Size(); ++i)
	{
		int filterTime = filters[i]->processingTime;
		totalProcessingTimes[i] += filterTime;
		totalFilterProcessingTimeThisFrame += filterTime;
	}
	totalProcessingTimes[filters.Size()] += totalFilterProcessingTimeThisFrame;
	totalProcessingTimes[filters.Size()+1] += pipelineTimeConsumption;
	++iterations;

	// If the output was not an image and this is the last in sequence, make sure to paint something renderable onto the output texture.
	if (filterToPaint)
	{
		filterToPaint->Paint(this);
	}
	return returnType;
}


// o-o
void CVPipeline::PrintProcessingTime()
{
	if (totalProcessingTimes.Size() != filters.Size() + 2)
		return;
	std::cout<<"\n\nCVPipeline filter processing times. Average presented over "<<iterations<<" iterations.";
#define FILL_NAME std::left<<std::setw(35)
#define FILL_CURR std::right<<std::setw(15)
#define FILL_AVER std::setprecision(3)<<std::fixed<<std::right<<std::setw(15)
	std::cout<<"\n\n"<<FILL_NAME<<"Filter name"<<FILL_CURR<<"Last iteration"<<FILL_AVER<<"Average";
	for (int i = 0; i < filters.Size(); ++i)
	{
		CVFilter * filter = filters[i];
		double average = double(totalProcessingTimes[i]) / iterations;
		std::cout<<"\n"<<FILL_NAME<<filter->name<<FILL_CURR<<filter->processingTime<<FILL_AVER<<average;		
	}
	std::cout<<"\n"<<FILL_NAME<<"Total "<<FILL_CURR<<totalFilterProcessingTimeThisFrame<<FILL_AVER<<(totalProcessingTimes[filters.Size()]) / (double)iterations;
	std::cout<<"\n"<<FILL_NAME<<"Total+Copy "<<FILL_CURR<<pipelineTimeConsumption<<FILL_AVER<<(totalProcessingTimes[filters.Size()+1]) / (double)iterations;


}


/** Converts target input-space coordinate (CV image co-ordinates) into output-space coordinates, 
	as is handled in the game engine and rendered out.
*/
Vector3f CVPipeline::InputSpaceCoordToOutputSpaceCoord(Vector3f imageSpaceCoord)
{
	/// Assume we are still and centered compared to the world-space.
	Vector3f size = initialInputSize;
	Vector3f center = Vector3f();
	
	
	// Origo to origo.
	Vector3f oToO(size.x, - size.y, 0);
	oToO *= 0.5f;
	// Add to the co-ordinate (or subtract?) so that the origos match up.
	Vector3f imageSpaceOrigoToWorldSpaceOrigo = Vector3f(0,0,0) - oToO;
	
	Vector3f worldSpaceCoord = imageSpaceOrigoToWorldSpaceOrigo + Vector3f(imageSpaceCoord.x, -imageSpaceCoord.y, 0);


	/// Multiply as needed, now that origos should be centered!
	Vector3f scaledWorldCoord = worldSpaceCoord.ElementDivision(outputProjectionRelativeSizeInInput);
	scaledWorldCoord.z = 0;
//	std::cout<<"\nWorld-space coordinate: "<<scaledWorldCoord;

	return scaledWorldCoord;
}

