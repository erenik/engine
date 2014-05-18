/// Emil Hedemalm
/// 2014-04-09
/// OpenCV Pipeline for handling input, filters, calculation-filters (working with points/blobs) and output.

#include "CVPipeline.h"
#include "Timer/Timer.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMUI.h"
#include <fstream>

CVPipeline::CVPipeline()
{
}
CVPipeline::~CVPipeline()
{
	filters.ClearAndDelete();
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
		filters.Add(filter);
	}
	// Done!
	return true;
}

// Clears all filters, calling OnDelete on them so that they may do proper clean-up.
void CVPipeline::Clear()
{
	while(filters.Size())
		DeleteFilterByIndex(0);
}


/// Current amount of filters.
int CVPipeline::Filters()
{
	return filters.Size();
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
	// And delete it.
	delete filter;
	return filter;
}

/** Takes input image and processes it through all filters and calc-filters. 
	Returns an index specifying the type of output generated or -1 if an error occured.
*/
int CVPipeline::Process(cv::Mat * i_initialInput)
{
	initialInput = i_initialInput;
	initialInput->copyTo(input);
	/// Copy to output too, in-case of empty pipeline.
	initialInput->copyTo(output);
	// Start timer
	Timer pipelineTimer;
	pipelineTimer.Start();
	Timer filterTimer;

	CVFilter * lastProcessedFilter = NULL;
	// To avoid unnecessary painting time.
	CVFilter * filterToPaint = NULL;

	returnType = CVReturnType::CV_IMAGE;
	for (int i = 0; i < filters.Size(); ++i)
	{
		CVFilter * filter = filters[i];
		// Skip temporarily disabled filters.
		if (!filter->enabled)
			continue;
		filterTimer.Start();
		returnType = filter->Process(this);
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
		if (returnType == -1){
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
		if (filter->status.Length())
			Graphics.QueueMessage(new GMSetUIs(filter->name+"Status", GMUI::TEXT, filter->status));
	}
	// Stop timer before painting!	
	pipelineTimer.Stop();
	pipelineTimeConsumption = pipelineTimer.GetMicro();

	// If the output was not an image and this is the last in sequence, make sure to paint something renderable onto the output texture.
	if (filterToPaint)
	{
		filterToPaint->Paint(this);
	}
		
	return returnType;
}

