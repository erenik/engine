/// Emil Hedemalm
/// 2014-04-09
/// Filters that have images as (both input and) output.

#include "CVImageFilters.h"
#include "CV/CVPipeline.h"
#include "String/StringUtil.h"

CVImageFilter::CVImageFilter(int id)
	: CVFilter(id)
{
	type = CVFilterType::IMAGE_FILTER;
};

void CVImageFilter::Paint(CVPipeline * pipe)
{
//	std::cout<<"\nCVImageFilter::Paint called. This should not happen?";
}


CVToChar::CVToChar()
	: CVImageFilter(CVFilterID::TO_CHAR)
{
}

int CVToChar::Process(CVPipeline * pipe)
{
	try {
		pipe->input.convertTo(pipe->output, CV_8UC1);
	} catch (...)
	{
		errorString = " D:";
		return -1;
	}
	return CVReturnType::CV_IMAGE;
}

CVGreyscaleFilter::CVGreyscaleFilter()
	: CVImageFilter(CVFilterID::GREYSCALE)
{
}

// Main processing function, sub-class and re-implement this.
int CVGreyscaleFilter::Process(CVPipeline * pipe)
{
	if (pipe->input.channels() == 1)
	{
		errorString = "image already monochrome";
		return CVReturnType::NO_OUTPUT;
	}
	cv::cvtColor(pipe->input, pipe->greyscaled, CV_BGR2GRAY);
	pipe->greyscaled.copyTo(pipe->output);
	return CVReturnType::CV_IMAGE;
}

CVScaleUpFilter::CVScaleUpFilter()
	: CVImageFilter(CVFilterID::SCALE_UP)
{
	scale = new CVFilterSetting("scale", 2.0f);
	settings.Add(scale);
}

// Main processing function, sub-class and re-implement this.
int CVScaleUpFilter::Process(CVPipeline * pipe)
{
	if (pipe->input.cols == 1 || pipe->input.rows == 1)
	{
		errorString = "image already too schmall! Stahp! D:";
		return CVReturnType::NO_OUTPUT;
	}
#ifdef WINDOWS
	cv::pyrUp(pipe->input, pipe->output, cv::Size(pipe->input.cols * scale->GetFloat(), pipe->input.rows * scale->GetFloat()));
#else
	assert(false && "Either fix includes in linux version or compile pyrUp");
#endif
	pipe->currentScale *= scale->GetFloat();
	pipe->currentScaleInv /= scale->GetFloat();
	return CVReturnType::CV_IMAGE;
}

CVScaleDownFilter::CVScaleDownFilter()
	: CVImageFilter(CVFilterID::SCALE_DOWN)
{
	scale = new CVFilterSetting("scale", 0.5f);
	settings.Add(scale);
}

// Main processing function, sub-class and re-implement this.
int CVScaleDownFilter::Process(CVPipeline * pipe)
{
	if (pipe->input.cols >= 2024 || pipe->input.rows >= 2024)
	{
		errorString = "image already huge. Stahp! ovo";
		return CVReturnType::NO_OUTPUT;
	}
	else if (pipe->input.cols <= 0 || pipe->input.rows <= 0)
	{
		errorString = "Image already too small.";
		return CVReturnType::NO_OUTPUT;
	}
	try {
#ifdef WINDOWS
		cv::pyrDown(pipe->input, pipe->output, cv::Size(pipe->input.cols * scale->GetFloat(), pipe->input.rows * scale->GetFloat()));
#else
		assert(false && "Either fix includes in linux version or compile pyrUp");
#endif
		pipe->currentScale *= scale->GetFloat();
		pipe->currentScaleInv /= scale->GetFloat();
		// Copy to scaledDown version of pipeline too, for longer pipes.
		pipe->output.copyTo(pipe->scaledDown);
	}
	catch(...)
	{
		errorString = "pyrDown failed";
		return CVReturnType::NO_OUTPUT;
	}
	return CVReturnType::CV_IMAGE;
}


CVCannyEdgeFilter::CVCannyEdgeFilter()
	: CVImageFilter(CVFilterID::CANNY_EDGE)
{
	edgeThreshold = new CVFilterSetting("edge threshold", 10.f);
	settings.Add(edgeThreshold);
	ratio = new CVFilterSetting("ratio", 3.f);
	settings.Add(ratio);
	kernelSize = new CVFilterSetting("kernel size", 3);
	settings.Add(kernelSize);

}

// Main processing function, sub-class and re-implement this.
int CVCannyEdgeFilter::Process(CVPipeline * pipe)
{
	
	// Make greyscale if not already?
	if (pipe->input.channels() > 1)
	{
		errorString = "Input must be single-channel.";
		return CVReturnType::NO_OUTPUT;
	}

	int max_lowThreshold = 100;
	
	int kernel_size = kernelSize->GetInt();
		
//	cv::Mat detected_edges;
	try {
		cv::Canny(pipe->input, pipe->output, edgeThreshold->GetFloat(), edgeThreshold->GetFloat() * ratio->GetFloat(), kernel_size * 2 + 1);
	}catch(...)
	{
		errorString = "Input settings not within acceptable range.";
		return CVReturnType::NO_OUTPUT;
	}

	/// Do something with the result, preparing for masking...
//	pipe->output = cv::Scalar::all(0);

	/// Copy using the detected edges as a mask.
//	pipe->input.copyTo(pipe->output, pipe->cannyOutput);

	// Copy out the output!
//	pipe->cannyOutput.copyTo(pipe->input);

	return CVReturnType::CV_IMAGE;
}


CVInvertFilter::CVInvertFilter()
	: CVImageFilter(CVFilterID::INVERT)
{

}

int CVInvertFilter::Process(CVPipeline * pipe)
{
	try {
		cv::Mat * mat = &pipe->input;
		int channels = mat->channels();
		int bytesPerChannel = mat->step / mat->cols;
		int type = mat->type();
		/// 1 for uchar (0-255), 2 for 32-bit float.
		int dataType = 0;
		switch(type)
		{
			case CV_32FC1:
			case CV_32FC2:
			case CV_32FC3:
			case CV_32FC4:
				dataType = DataType::FLOAT;
				break;
			default:
				dataType = DataType::UNSIGNED_CHAR;
				break;
		}
		// Allocate output to be same size as input
		pipe->output.create(mat->rows, mat->cols, mat->type());
		/// Then load over the data!
		unsigned char * cData = (unsigned char*) (mat->data);
		float * fData = (float *) mat->data;
		unsigned char * outputData = (unsigned char*) (pipe->output.data);
		float * fOutputData = (float*) (pipe->output.data);
		int yStep, xStep, cStep;
		// Manual invert of the entire image.
		// For UCHAR
		if (dataType == DataType::UNSIGNED_CHAR)
		{
			for (int y = 0; y < mat->rows; ++y)
			{
				yStep = y * mat->cols * channels;
				for (int x = 0; x < mat->cols; ++x)
				{
					xStep = x * channels;
					for (int c = 0; c < channels; ++c)
					{
						int index = yStep + xStep + c;
						outputData[index] = 255 - cData[index];
					}
				}
			}
		}
		/// .. and for FLOAT
		else if (dataType == DataType::FLOAT)
		{
			/// Use indices!
			for (int y = 0; y < mat->rows; ++y)
			{
				yStep = y * mat->cols * channels;
				for (int x = 0; x < mat->cols; ++x)
				{
					xStep = x * channels;
					for (int c = 0; c < channels; ++c)
					{
						int index = yStep + xStep + c;
						fOutputData[index] = 255 - fData[index];
					}
				}
			}
		}

	} catch(...)
	{
		errorString = "Inverting failed.";
		return CVReturnType::NOTHING;
	}
	return CVReturnType::CV_IMAGE;
}


CVHarrisCornerFilter::CVHarrisCornerFilter()
	: CVImageFilter(CVFilterID::HARRIS_CORNER)
{
	blockSize = new CVFilterSetting("Block size", 5);
	kSize = new CVFilterSetting("k size", 5);
	k = new CVFilterSetting("k", 0.04f);

	settings.Add(blockSize);
	settings.Add(kSize);
	settings.Add(k);

	List<String> lines;
	lines.Add("A threshold filter is usually required in order");
	lines.Add("to convert the 32-bit signed floating point values ");
	lines.Add("into observable byte texture units (0-255).");
	about = MergeLines(lines, "\n");

	//CVFilterSetting * blockSize, * kSize, * k;
	/*	cornerHarrisBlockSize = 5;
	cornerHarrisKSize = 5;
	cornerHarrisK = 0.04f;
	*/
}

int CVHarrisCornerFilter::Process(CVPipeline * pipe)
{
	// Make greyscale if not already.
	if (pipe->input.channels() > 1)
	{
		errorString = "HarrisCorner requires greyscale input.";
		return CVReturnType::NO_OUTPUT;
	}
	else if (!(kSize->GetInt() & 1))
	{
		errorString = "kSize must be odd.";
		return CVReturnType::NOTHING;
	}
	else if (kSize->GetInt() > 31)
	{
		errorString = "kSize may not go above 31";
		return CVReturnType::NOTHING;
	}
	/// Result is a 32-bit floating point signed image
	cv::Mat detected_corners;
	try {
		cv::cornerHarris(pipe->input, pipe->output, blockSize->GetInt(), kSize->GetInt(), k->GetFloat());
	} catch (...) 
	{
		errorString = "Bad settings.";
		return CVReturnType::NOTHING;
	}
	// Do some magic... to make the image renderable/visible?
//	cv::Mat thresholdCulledCorners;
	//threshold(detected_corners, thresholdCulledCorners, 0.00001, 255, cv::THRESH_BINARY);

	///// Copy to result image for display
//	thresholdCulledCorners.copyTo(pipe->output);
	
	// Display it
	return CVReturnType::CV_IMAGE;
}


CVGaussianBlurFilter::CVGaussianBlurFilter()
	: CVImageFilter(CVFilterID::GAUSSIAN_BLUR)
{
	blurSize = new CVFilterSetting("Blur size", 3);
	blurSizeXY = new CVFilterSetting("Blur size XY", Vector2i(3,3));
	type = new CVFilterSetting("Type", 0);
	settings.Add(3,
		blurSize, blurSizeXY, type);
}

int CVGaussianBlurFilter::Process(CVPipeline * pipe)
{
	int blur = blurSize->GetInt() * 2 + 1;
	try {
		int blurType = type->GetInt();
		switch(blurType)
		{
			case 0:
				cv::GaussianBlur(pipe->input, pipe->output, cv::Size(blur, blur), 0, 0);
				pipe->output.copyTo(pipe->blurred);
				break;
			case 1:
			{
				Vector2i size = blurSizeXY->GetVec2i();
				size = size * 2 + Vector2i(1,1);
				cv::GaussianBlur(pipe->input, pipe->output, cv::Size(size.x, size.y), 0, 0);
				pipe->output.copyTo(pipe->blurred);
				break;
			}
			default:
				return CVReturnType::CV_ERROR;
		}
	} catch(...)
	{
		std::cout<<"Blur failed.";
		return CVReturnType::NOTHING;
	}
	return CVReturnType::CV_IMAGE;
}



CVThresholdFilter::CVThresholdFilter()
	: CVImageFilter(CVFilterID::THRESHOLD)
{
	threshold = new CVFilterSetting("Threshold", 0.00001f);
	settings.Add(threshold);
	maxValue = new CVFilterSetting("Max value", 255.f);
	settings.Add(maxValue);
	type = new CVFilterSetting("Type", cv::THRESH_BINARY);
	settings.Add(type);

	List<String> lines;
	lines.Add("Used to narrow scope/limits of values.");
	lines.Add("Type values:");
	lines.Add("0 - THRESH_BINARY");
	lines.Add("1 - THRESH_BINARY_INV");
	lines.Add("2 - THRESH_TRUNC");
	lines.Add("3 - THRESH_TOZERO");
	lines.Add("4 - THRESH_TOZERO_INV");
	about = MergeLines(lines, "\n");
}

int CVThresholdFilter::Process(CVPipeline * pipe)
{
	// Do some magic... to make the image renderable/visible?
	try {
		cv::threshold(pipe->input, pipe->output, threshold->GetFloat(), maxValue->GetFloat(), type->GetInt());
	} catch (...)
	{
		errorString = "Threshold failed.";
		return CVReturnType::NOTHING;
	}
	return CVReturnType::CV_IMAGE;
}

CVAdaptiveThreshold::CVAdaptiveThreshold()
	: CVImageFilter(CVFilterID::ADAPTIVE_THRESHOLD)
{
	relativeThreshold = new CVFilterSetting("Relative threshold", 0.5f);
	settings.Add(relativeThreshold);
	minimumThreshold = new CVFilterSetting("Minimum threshold", 20.f);
	settings.Add(minimumThreshold);
	minimumRange = new CVFilterSetting("Minimum range", 20.f);
	settings.Add(minimumRange);
	about = "Uses a threshold relative to the minimum and\nmaximum detected values in the input image.\n\
			Minimum range defines the minimum difference \n(contrast) between the max and min values within the image.";
}
int CVAdaptiveThreshold::Process(CVPipeline * pipe)
{
	// Find min/max.
	double min, max;
	cv::minMaxLoc(pipe->input, &min, &max);
	double span = max - min;
	double currentThreshold = span * relativeThreshold->GetFloat() + min;
	
	if (currentThreshold < minimumThreshold->GetFloat())
	{
		currentThreshold = minimumThreshold->GetFloat();
	}
	if (span < minimumRange->GetFloat())
	{
		currentThreshold = max + 1.f;
	}

	// Do some magic... to make the image renderable/visible?
	try {
		cv::threshold(pipe->input, pipe->output, currentThreshold, 255, cv::THRESH_BINARY);
	} catch (...)
	{
		errorString = "Threshold failed.";
		return CVReturnType::NOTHING;
	}
	return CVReturnType::CV_IMAGE;
}

CVPerPixelAdaptiveThreshold::CVPerPixelAdaptiveThreshold()
	: CVImageFilter(CVFilterID::PER_PIXEL_ADAPTIVE_THRESHOLD)
{
//		CVFilterSetting * maxValue, * adaptiveMethod, * thresholdType,
//		* blockSize, * c;
	maxValue = new CVFilterSetting("Max value", 255.f);
	adaptiveMethod = new CVFilterSetting("Adaptive Method", cv::ADAPTIVE_THRESH_MEAN_C);
	thresholdType = new CVFilterSetting("Threshold", cv::THRESH_BINARY);
	blockSize = new CVFilterSetting("Block size", 0);
	c = new CVFilterSetting("c", 0.0f);
	settings.Add(5, maxValue, adaptiveMethod, thresholdType, blockSize, c);
}
int CVPerPixelAdaptiveThreshold::Process(CVPipeline * pipe)
{
		// Do some magic... to make the image renderable/visible?
	try {
		cv::adaptiveThreshold(pipe->input, pipe->output, 
			maxValue->GetFloat(), adaptiveMethod->GetInt(), 
			thresholdType->GetInt(), blockSize->GetInt()*2 + 1, c->GetFloat());
	} catch (...)
	{
		errorString = "Threshold failed.";
		return CVReturnType::NOTHING;
	}
	return CVReturnType::CV_IMAGE;
}



CVAbsFilter::CVAbsFilter()
	: CVImageFilter(CVFilterID::ABS)
{
	List<String> lines;
	lines.Add("Calculates absolute-values for every pixel and channel.");
	lines.Add("Meant to be used primarily on signed floating point");
	lines.Add("images.");
	about = MergeLines(lines, "\n");
}
int CVAbsFilter::Process(CVPipeline * pipe)
{
		try {
		cv::Mat * mat = &pipe->input;
		int channels = mat->channels();
		int bytesPerChannel = mat->step / mat->cols;
		int type = mat->type();
		/// 1 for uchar (0-255), 2 for 32-bit float.
		int dataType = 0;
		switch(type)
		{
			case CV_32FC1:
			case CV_32FC2:
			case CV_32FC3:
			case CV_32FC4:
				dataType = DataType::FLOAT;
				break;
			default:
				dataType = DataType::UNSIGNED_CHAR;
				break;
		}
		// Allocate output to be same size as input
		pipe->output.create(mat->rows, mat->cols, mat->type());
		/// Then load over the data!
		unsigned char * cData = (unsigned char*) (mat->data);
		float * fData = (float *) mat->data;
		unsigned char * outputData = (unsigned char*) (pipe->output.data);
		float * fOutputData = (float*) (pipe->output.data);
		int yStep, xStep, cStep;
		// Manual invert of the entire image.
		// For UCHAR
		if (dataType == DataType::UNSIGNED_CHAR)
		{
			for (int y = 0; y < mat->rows; ++y)
			{
				yStep = y * mat->cols * channels;
				for (int x = 0; x < mat->cols; ++x)
				{
					xStep = x * channels;
					for (int c = 0; c < channels; ++c)
					{
						int index = yStep + xStep + c;
						outputData[index] = 255 - cData[index];
					}
				}
			}
		}
		/// .. and for FLOAT
		else if (dataType == DataType::FLOAT)
		{
			/// Use indices!
			for (int y = 0; y < mat->rows; ++y)
			{
				yStep = y * mat->cols * channels;
				for (int x = 0; x < mat->cols; ++x)
				{
					xStep = x * channels;
					for (int c = 0; c < channels; ++c)
					{
						int index = yStep + xStep + c;
						fOutputData[index] = AbsoluteValue(fData[index]);
					}
				}
			}
		}

	} catch(...)
	{
		errorString = "Inverting failed.";
		return CVReturnType::NOTHING;
	}
	return CVReturnType::CV_IMAGE;
}

CVColorFilter::CVColorFilter()
	: CVImageFilter(CVFilterID::COLOR_FILTER)
{
	colorToFilter = new CVFilterSetting("Color to filter", Vector3f(1,1,1));
	colorToReplace = new CVFilterSetting("Color to replace", Vector3f(0,0,0));
	mode = new CVFilterSetting("Mode", 0);
	settings.Add(colorToFilter);
	settings.Add(colorToReplace);
	settings.Add(mode);
}
int CVColorFilter::Process(CVPipeline * pipe)
{

	return CVReturnType::CV_IMAGE;
}

CVExtractChannels::CVExtractChannels()
	: CVImageFilter(CVFilterID::EXTRACT_CHANNELS)
{
	this->channelToDisplay = new CVFilterSetting("Channel to display", 0);
	settings.Add(channelToDisplay);

	List<String> lines;
	lines += "Extracts Hue, Light and Saturation channels.";
	lines += "0 = Hue";
	lines += "1 = Light";
	lines += "2 = Saturation";
	about = MergeLines(lines, "\n");
}
int CVExtractChannels::Process(CVPipeline * pipe)
{
	if (pipe->input.channels() == 1)
	{
		// Convert as needed.
		cv::cvtColor(pipe->input, pipe->output, CV_GRAY2BGR);
		// Copy
		pipe->output.copyTo(pipe->input);
	}

	if (pipe->input.channels() != 3)
	{
		errorString = "Unexpected amount of channels. Should be 3.";
		return -1;
	}


	// Convert to lab
	cv::cvtColor(pipe->input, pipe->output, CV_BGR2HLS_FULL);

	// Extract saturation-channel
	std::vector<cv::Mat> channels;
	cv::split(pipe->output, channels);
	pipe->hue = channels[0];
	pipe->value = channels[1];
	pipe->saturation = channels[2];
	return CVReturnType::CV_CHANNELS;
}
// Custom paint method.
void CVExtractChannels::Paint(CVPipeline * pipe)
{
	
	switch(channelToDisplay->GetInt())
	{
	case 0:
		pipe->output = pipe->hue;
		break;
	case 1:
		pipe->output = pipe->value;
		break;
	case 2:
		pipe->output = pipe->saturation;
		break;
	}	
}

CVHueFilter::CVHueFilter()
	: CVImageFilter(CVFilterID::HUE_FILTER)
{
	targetHue = new CVFilterSetting("Target hue", 0);
	range = new CVFilterSetting("Hue range", 20);
	settings.Add(targetHue);
	settings.Add(range);
	about = "Filters by hue. Hue values are between 0-255 and are considered\nin a circular manner. (0 and 255 next to each other)";
}

struct Range 
{
	Range(int start = -1, int stop = -1) : start(start), stop(stop) {};
	int start, stop;
};

List<Range> GetSubRangesForCircularArray(Range & range, int arrayStart, int arrayStop)
{
	int arraySize = arrayStop - arrayStart;
	List<Range> subRanges;
	// Part exceeding bottom.
	if (range.start < arrayStart)
	{
		// Create bottom-part.
		Range bottomPart(range.start + arraySize, arrayStop);
		subRanges.Add(bottomPart);
	}
	// Part exceeding top.
	if (range.stop > arrayStop)
	{
		Range topPart(arrayStart, range.stop - arraySize);
		subRanges.Add(topPart);
	}
	// Extremes may be outside of range, but it won't matter, as long as it includes the necessary range as well.
	subRanges.Add(Range(range.start, range.stop));
	return subRanges;
}

int CVHueFilter::Process(CVPipeline * pipe)
{
	pipe->output = pipe->hue;
	/// Go through the values.
	/// Then load over the data!
	unsigned char * cData = (unsigned char*) (pipe->output.data);
	unsigned char * outputData = (unsigned char*) (pipe->output.data);
	int yStep, xStep, cStep;
	int channels = pipe->output.channels();

	// There will be at most two unique intervals since a circular array is used for hues.
	Range targetRange(targetHue->GetInt() - range->GetInt(), targetHue->GetInt() + range->GetInt());
	List<Range> subRanges = GetSubRangesForCircularArray(targetRange, 0, 255);

	// Manual invert of the entire image.
	int index, saturation;
	bool within;
	// For UCHAR
	for (int y = 0; y < pipe->output.rows; ++y)
	{
		yStep = y * pipe->output.cols * channels;
		for (int x = 0; x < pipe->output.cols; ++x)
		{
			xStep = x * channels;
			/// Fetch colors.
			index = yStep + xStep;
			saturation = cData[index];
			
			
			within = false;
			for (int i = 0; i < subRanges.Size(); ++i)
			{
				Range & range = subRanges[i];
				if (saturation >= range.start && saturation <= range.stop)
				{
					within = true;
					break;
				}
			}
			// Mark it 
			if (within)
				cData[index] = 255;
			else 
				cData[index] = 0;
		}
	}
	return CVReturnType::CV_IMAGE;
}

CVSaturationFilter::CVSaturationFilter()
	: CVImageFilter(CVFilterID::SATURATION_FILTER)
{
	targetSaturation = new CVFilterSetting("Target saturation", 0);
	scope = new CVFilterSetting("Scope/range", 30);
	settings.Add(targetSaturation);
	settings.Add(scope);

	about = "Paints black the pixels whose saturatoin are within target range.";
}
int CVSaturationFilter::Process(CVPipeline * pipe)
{
	// Ensure the image is greyscale.
	if (pipe->input.channels() != 1)
	{
		errorString = "Must be single-channel.";
		return -1;
	}
	// Ensure more stuff as needed.
	// ...
	// Go through every pixel
	unsigned char * cData = (unsigned char*) (pipe->saturation.data);
	if (!cData)
	{
		errorString = "No available saturation data.";
		return -1;
	}
	unsigned char * outputData = (unsigned char*) (pipe->output.data);
	int yStep, xStep, cStep;
	int channels = pipe->input.channels();
	// Copy input to output
	pipe->input.copyTo(pipe->output);
	// For UCHAR
	for (int y = 0; y < pipe->input.rows; ++y)
	{
		yStep = y * pipe->input.cols * channels;
		for (int x = 0; x < pipe->input.cols; ++x)
		{
			xStep = x * channels;
			/// Fetch colors.
			int index = yStep + xStep;
			int value = cData[index];

			// Within. Remove?
			if (value >= targetSaturation->GetInt() - scope->GetInt() &&
				value <= targetSaturation->GetInt() + scope->GetInt())
			{
				// Replace with saturation
				outputData[index] = 0;	
			}
		}
	}

	return CVReturnType::CV_IMAGE;

};


CVValueFilter::CVValueFilter()
	: CVImageFilter(CVFilterID::VALUE_FILTER)
{
		//CVFilterSetting * targetValue, * scope, * replacementColor;
	targetValue = new CVFilterSetting("Target value", 255);
	scope = new CVFilterSetting("Scope/range", 30);
	replacementValue = new CVFilterSetting("Repalcement value", 0);
//	replacementColor = bnew
	settings.Add(3, targetValue, scope, replacementValue);

	about = "Paints black the pixels whose value are within target range.";
}
int CVValueFilter::Process(CVPipeline * pipe)
{
	// Ensure the image is greyscale.
	if (pipe->input.channels() != 1)
	{
		errorString = "Must be single-channel.";
		return -1;
	}
	// Ensure more stuff as needed.
	// ...
	// Go through every pixel
	unsigned char * cData = (unsigned char*) (pipe->value.data);
	if (!cData)
		cData = (unsigned char*) pipe->blurred.data;
	if (!cData)
		cData = (unsigned char*) pipe->greyscaled.data;
	if (!cData && pipe->input.channels() == 1)
		cData = (unsigned char *) pipe->input.data;
	if (!cData)
	{
		errorString = "No available value data.";
		return -1;
	}
	unsigned char * outputData = (unsigned char*) (pipe->output.data);
	int yStep, xStep, cStep;
	int channels = pipe->input.channels();
	// Copy input to output
	pipe->input.copyTo(pipe->output);
	int repVal = replacementValue->GetInt();
	// For UCHAR
	for (int y = 0; y < pipe->input.rows; ++y)
	{
		yStep = y * pipe->input.cols * channels;
		for (int x = 0; x < pipe->input.cols; ++x)
		{
			xStep = x * channels;
			/// Fetch colors.
			int index = yStep + xStep;
			int value = cData[index];

			// Within. Remove?
			if (value >= targetValue->GetInt() - scope->GetInt() &&
				value <= targetValue->GetInt() + scope->GetInt())
			{
				// Replace with target value
				outputData[index] = repVal;	
			}
		}
	}

	return CVReturnType::CV_IMAGE;
}




CVRemoveBackgroundFilter::CVRemoveBackgroundFilter()
	: CVImageFilter(CVFilterID::REMOVE_BACKGROUND)
{
	extractBackground = new CVFilterSetting("Extract background now");
	settings.Add(extractBackground);
	automatic = new CVFilterSetting("Automatic", true);
	settings.Add(automatic);
	framesToConsider = new CVFilterSetting("Frames to consider", 20);
	settings.Add(framesToConsider);
	threshold = new CVFilterSetting("Threshold", 10.1f);
	settings.Add(threshold);

	about = "Treshold being ratio which pixels in general deviate from the current frame.";

	framesPassed = 0;
}
int CVRemoveBackgroundFilter::Process(CVPipeline * pipe)
{
	// Convert input to 16-bit for usage.
	cv::Mat convertedInput;
	pipe->input.convertTo(convertedInput, CV_16UC3);

	if (pipe->input.rows == 0)
		return -1;

	try {
		// If automatic detection, check if we should extract background again.
		if (automatic->GetBool())
		{
			// Check that the sum matrix is the same type as the input, if not re-allocate it first and reset variables.
			if (convertedInput.type() != framesTotal.type() ||
				convertedInput.rows != framesTotal.rows)
			{

				// Convert to 32-bit integer storage though!
				int channels = pipe->input.channels();
				framesTotal = cv::Mat::zeros(pipe->input.rows, pipe->input.cols, convertedInput.type());
			
				framesPassed = 0;
			}
			// Add current frame
			cv::add(convertedInput, framesTotal, framesTotal);
			framesPassed++;

			// When 20 accumulated frames.
			if (framesPassed >= framesToConsider->GetInt())
			{
				cv::Scalar sums;
				float sumTotal = 0;
				// Check sums...
				sums = cv::sum(framesTotal);
				for (int i = 0; i < 4; ++i)
				{
					sumTotal += sums[i];
					if (i == 0)
					;//	std::cout<<"\nSum before mult: "<<sums[i];
				}
			
				// Calculate average
				float multiplier = 1.f / framesPassed;
				framesTotal = framesTotal * multiplier; // .mul(framesTotal, multiplier);
				
				// Check sums...
				sums = cv::sum(framesTotal);
				sumTotal = 0;
				for (int i = 0; i < 4; ++i)
				{
					sumTotal += sums[i];
					if (i == 0)
					;//	std::cout<<"\nSum post mult: "<<sums[i];
				}

				// Reset it so we wait another 20 frames before next check...
				framesPassed = 0;
			
				// Compare with current frame
				cv::absdiff(convertedInput, framesTotal, diffMat);
		
				// If total diff is below threshold
				sums = cv::sum(diffMat);
				sumTotal = 0;
				for (int i = 0; i < 4; ++i)
				{
					sumTotal += sums[i];
					if (i == 0)
						std::cout<<"\nSum: "<<sums[i];
				}
				sumTotal /= pipe->input.rows * pipe->input.cols;
				std::cout<<" total: "<<sumTotal;
				// Divide with amount of pixels in the image.
				// Reset background when there is no noticable movement within, deemed by the threshold!
				if (sumTotal < threshold->GetFloat()){
					extractBackground->SetBool(true);
				}
				// Set zeroes in framesTotal.
				framesTotal.setTo(0);
			}
		}
	} catch(...)
	{
		std::cout<<"Error in automatic background detection.";
		return -1;
	}

	if(extractBackground->GetBool())
	{
		pipe->input.copyTo(background);
		errorString = "Background saved from input.";
		extractBackground->SetBool(false);
//		return -1;
	}
	int inputP, backgroundP, outputP;
		
	/// Use provided background!
	try {
		bool displayDiff = false;
		if (displayDiff)
		{
			diffMat.copyTo(pipe->output);
		}
		else
		{	
			cv::absdiff(pipe->input, background, pipe->output);
			inputP = pipe->input.data[0];
			backgroundP = background.data[0];
			outputP = pipe->output.data[0];
		}
	}catch(...)
	{
		errorString = "Try to extract background again.";
		return -1;
	}

	return CVReturnType::CV_IMAGE;
}


CVErode::CVErode()
	: CVImageFilter(CVFilterID::ERODE)
{
	type = new CVFilterSetting("Erosion type", 0);
	size = new CVFilterSetting("Erosion size", 0);
	settings.Add(type);
	settings.Add(size);
}
int CVErode::Process(CVPipeline * pipe)
{
	int erosionType;
	switch(type->GetInt())
	{
	case 0: default:
		erosionType = cv::MORPH_RECT;
		break;
	case 1:
		erosionType = cv::MORPH_CROSS;
		break;
	case 2:
		erosionType = cv::MORPH_ELLIPSE;
		break;
	}
	try {
		cv::Mat erosionElement = cv::getStructuringElement(
									erosionType,
									cv::Size(size->GetInt() * 2 + 1, size->GetInt() * 2 + 1),
									cv::Point(size->GetInt(), size->GetInt())
								);
		cv::erode(pipe->input, pipe->output, erosionElement);
	} catch(...)
	{
		return -1;
	}
	return CVReturnType::CV_IMAGE;
}


CVDilate::CVDilate()
	: CVImageFilter(CVFilterID::DILATE)
{
	type = new CVFilterSetting("Dilation type", 0);
	size = new CVFilterSetting("Dilation size", 0);
	settings.Add(type);
	settings.Add(size);
}
int CVDilate::Process(CVPipeline * pipe)
{
	int dilationType;
	switch(type->GetInt())
	{
	case 0: default:
		dilationType = cv::MORPH_RECT;
		break;
	case 1:
		dilationType = cv::MORPH_CROSS;
		break;
	case 2:
		dilationType = cv::MORPH_ELLIPSE;
		break;
	}
	try {
		cv::Mat erosionElement = cv::getStructuringElement(
									dilationType,
									cv::Size(size->GetInt() * 2 + 1, size->GetInt() * 2 + 1),
									cv::Point(size->GetInt(), size->GetInt())
								);
		cv::dilate(pipe->input, pipe->output, erosionElement);
	} catch(...)
	{
		CATCH_EXCEPTION("CVDilate");
		return -1;
	}
	return CVReturnType::CV_IMAGE;
}



CVFill3::CVFill3()
	: CVImageFilter(CVFilterID::FILL_THREE)
{
	maxDistanceToCheck = new CVFilterSetting("Max distance", 200);
	settings.Add(maxDistanceToCheck);
}
int CVFill3::Process(CVPipeline * pipe)
{
	// For every pixel...
	// Ensure the image is greyscale.
	if (pipe->input.channels() != 1)
	{
		errorString = "Must be single-channel.";
		return -1;
	}
	// Ensure more stuff as needed.
	// Go through every pixel
	unsigned char * cData = (unsigned char*) (pipe->input.data);
	int yStep, xStep, cStep;
	int channels = pipe->input.channels();
	// Copy input to output
	pipe->input.copyTo(pipe->output);
	unsigned char * outputData = (unsigned char*) (pipe->output.data);
	int columns = pipe->input.cols;
	int rows = pipe->input.rows;
	// For UCHAR
	for (int y = 0; y < pipe->input.rows; ++y)
	{
		yStep = y * columns * channels;
		for (int x = 0; x < columns; ++x)
		{
			xStep = x * channels;
			/// Fetch colors.
			int index = yStep + xStep;
			int value = cData[index];
			// If white, skip it.
			if (value)
				continue;

			int neighbours = 0;
			// Check x-wise.. rightward!
			for (int pixelX = x; pixelX < columns && pixelX < x + maxDistanceToCheck->GetInt(); ++pixelX)
			{
				if (cData[yStep + pixelX * channels])
				{
					neighbours++;
					break;
				}
			}
			// Check x-wise.. westward!!
			for (int pixelX = x; pixelX >= 0 && pixelX > x - maxDistanceToCheck->GetInt(); --pixelX)
			{
				if (cData[yStep + pixelX * channels])
				{
					neighbours++;
					break;
				}
			}
			// Check y-wise.. upwards!!!
			for (int pixelY = y; pixelY >= 0 && pixelY > y - maxDistanceToCheck->GetInt(); --pixelY)
			{
				// Multiplying channels could be omitted due to the image being single-channel..
				if (cData[pixelY * columns * channels + xStep])
				{
					neighbours++;
					break;
				}
			}

			// Check y-wise.. down o.o
			for (int pixelY = y; pixelY < rows && pixelY < y + maxDistanceToCheck->GetInt(); ++pixelY)
			{
				// Multiplying channels could be omitted due to the image being single-channel..
				if (cData[pixelY * columns * channels + xStep])
				{
					neighbours++;
					break;
				}
			}

			// If stuff, paint it
			if (neighbours >= 3)
			{
				outputData[index] = 155;
			}
		}
	}
	return CVReturnType::CV_IMAGE;
}


// CVExtractLABColors();
// virtual int Process(CVPipeline * pipe);


CVRenderRegionOfInterest::CVRenderRegionOfInterest()
	: CVImageFilter(CVFilterID::RENDER_REGION_OF_INTEREST)
{

	topLeftCorner = new CVFilterSetting("Top left", Vector3f());
	bottomRightCorner = new CVFilterSetting("Bottom right", Vector3f());
	settings.Add(2, topLeftCorner, bottomRightCorner);
	about = "Left click in the main window to set top-left corner.\nRight-click for bottom-right corner";
}
int CVRenderRegionOfInterest::Process(CVPipeline * pipe)
{
	if (topLeftCorner->HasChanged())
	{
		pipe->outputTopLeftAsDetectedInInput = topLeftCorner->GetVec3f();
	}
	else
	{
		// Store current value so that it may be saved.
		topLeftCorner->SetVec3f(pipe->outputTopLeftAsDetectedInInput, true);
	}
	if (bottomRightCorner->HasChanged())
	{
		pipe->outputBottomRightAsDetectedInInput = bottomRightCorner->GetVec3f();
	}
	else 
	{
		bottomRightCorner->SetVec3f(pipe->outputBottomRightAsDetectedInInput, true);
	}


	

	// Fetch original input?
	pipe->initialInput.copyTo(pipe->output);

	Vector3f topLeft = pipe->outputTopLeftAsDetectedInInput;
	Vector3f bottomRight = pipe->outputBottomRightAsDetectedInInput;
	Vector3f size = bottomRight - topLeft;
	// Paint the output.
//	cv::Rect rect(topLeft.x, topLeft.y, size.x, size.y);

//	cv::rectangle(pipe->output, rect, cv::Scalar(255,255,255,255));


	Vector3f inputSize = pipe->initialInputSize;
	pipe->outputProjectionRelativeSizeInInput = size.ElementDivision(inputSize);

	Vector3f projectorOutputSize = pipe->projectorResolution;
	Vector3f projectorResolutionRelativeToInputSize = projectorOutputSize.ElementDivision(inputSize);


	return CVReturnType::CV_IMAGE;
}

/// Build-in messaging system. Used for advanced applications to communicate with the game engine properly.
void CVRenderRegionOfInterest::ProcessMessage(Message * message)
{
	
}


/** Main painting function. This will be called on the last processed filter in order to get a renderable result.
	Separated from main processing since painting can take an unnecessary long amount of time to complete.
*/
void CVRenderRegionOfInterest::Paint(CVPipeline * pipe)
{
	if (previousFilter)
	{
		previousFilter->Paint(pipe);
	}

	Vector3f topLeft = pipe->outputTopLeftAsDetectedInInput;
	Vector3f bottomRight = pipe->outputBottomRightAsDetectedInInput;
	Vector3f size = bottomRight - topLeft;
	// Paint the output.
	cv::Rect rect(topLeft.x, topLeft.y, size.x, size.y);
	cv::rectangle(pipe->output, rect, cv::Scalar(0,255,0,255));
}



CVRegionOfInterestFilter::CVRegionOfInterestFilter()
	: CVImageFilter(CVFilterID::REGION_OF_INTEREST_FILTER)
{
// CVFilterSetting * topLeftCorner, * bottomRightCorner;
	topLeftCorner = new CVFilterSetting("Top left", Vector3f());
	bottomRightCorner = new CVFilterSetting("Bottom right", Vector3f(800,600));
	fillColor = new CVFilterSetting("Fill color", Vector3f());
	replaceInitialInput = new CVFilterSetting("Replace initial input", true);
	settings.Add(4, topLeftCorner, bottomRightCorner, fillColor, replaceInitialInput);
}

int CVRegionOfInterestFilter::Process(CVPipeline * pipe)
{
	// Paint 4 quads, surroinding the area we want to isolate.

	pipe->input.copyTo(pipe->output);

	Vector3f c = fillColor->GetVec3f();
	cv::Scalar color(c.x, c.y, c.z);

	// Left side
	Vector3f topLeft = topLeftCorner->GetVec3f(),
		bottomRight = bottomRightCorner->GetVec3f();

	Vector2f roiSize = bottomRight - topLeft;
	Vector2i imageSize = pipe->inputSize;

	ClampFloat(roiSize.x, 0, imageSize.x);
	ClampFloat(roiSize.y, 0, imageSize.y);

	// Crop it.
	if (true)
	{
		cv::Rect regionOfInterest(topLeft.x, topLeft.y, roiSize.x, roiSize.y);
		cv::Mat cropped = pipe->input(regionOfInterest);
		// Copy it since this region of interest thingy is but temporary!
		cropped.copyTo(pipe->output);
		if (replaceInitialInput->GetBool())
		{
			// Replace initial input within the pipeline, recalculating stuff.
			pipe->SetInitialInput(cropped);
		}
	}
	// Render lol-version..
	if (false)
	{
		// Left side..
		cv::rectangle(pipe->output, cv::Rect(0, 0, topLeft.x, imageSize.y), color, CV_FILLED);
		// Right side
		cv::rectangle(pipe->output, cv::Rect(bottomRight.x, bottomRight.y, imageSize.x, imageSize.y), color, CV_FILLED);
		// BOTTOM!.
		cv::rectangle(pipe->output, cv::Rect(0, bottomRight.y, imageSize.x, imageSize.y), color, CV_FILLED);
	}

	return CVReturnType::CV_IMAGE;
}


CVLoadInitialInput::CVLoadInitialInput()
	: CVImageFilter(CVFilterID::LOAD_INITIAL_INPUT)
{
	adjustedInput = new CVFilterSetting("Adjusted input", 0);
	about = "Input:\n0 - initialn\n1 - Greyscaled (if done earlier)\n2 - Scaled down (if done earlier)";
	settings.Add(adjustedInput);
}
int CVLoadInitialInput::Process(CVPipeline * pipe)
{
	switch(adjustedInput->GetInt())
	{
		case 1:
			pipe->greyscaled.copyTo(pipe->output);
			break;
		case 2:
			pipe->scaledDown.copyTo(pipe->output);
			break;
		default:
			pipe->initialInput.copyTo(pipe->output);
	}
	return CVReturnType::CV_IMAGE;
}


CVOpticalFlowFilter::CVOpticalFlowFilter()
	: CVImageFilter(CVFilterID::OPTICAL_FLOW_FILTER)
{
	matrixSize = new CVFilterSetting("Matrix size", 3);
	increasePerFlow = new CVFilterSetting("Inrease per flow", 5);
	settings.Add(2, matrixSize, increasePerFlow);
}
int CVOpticalFlowFilter::Process(CVPipeline * pipe)
{
	// require greyscale input
	if (pipe->input.type() != CV_8UC1)
	{
		errorString = "Greyscale required";
		return -1;
	}
	assert(pipe->input.size() == pipe->output.size());
	// Based on the output of the optical flow LK (Lucas-Kanade) filter.
	cv::Mat & image = pipe->output;
	unsigned char * data = image.data;
	int size = image.cols * image.rows * image.channels();
	
	// First paint the whole image half of what it was.
	for (int i = 0; i < size; ++i)
	{
		data[i] = data[i] * 0.5f;
	}

	int matSize = matrixSize->GetInt();
	int inc = increasePerFlow->GetInt();
	for (int i = 0; i < pipe->opticalFlowPoints.Size(); ++i)
	{
		OpticalFlowPoint & point = pipe->opticalFlowPoints[i];

		// Paint some stuff in the image based on this.
		Vector2i position = point.position;
		int incByVel = point.offset.Length();

#define Maximum(a,b) ((a)>(b)? a : b)
#define Minimum(a,b) ((a)<(b)? a : b)
		for (int x = Maximum(position.x - matSize, 0); x < position.x + matSize && x < image.cols; ++x)
		{
			for (int y = Maximum(position.y - matSize, 0); y < position.y + matSize && y < image.rows; ++y)
			{
				int pixelIndex = (y * image.cols + x);
			
				if (pixelIndex > size)
					continue;
				int oldValue = data[pixelIndex];
				int newValue = Minimum((oldValue + inc + incByVel * 0.01f), 255);
				data[pixelIndex] = newValue;
				//	data
				//pipe->output.
			}
		}
	}
	return CVReturnType::CV_IMAGE;
}


CVContrastBrightnessFilter::CVContrastBrightnessFilter()
	: CVImageFilter(CVFilterID::CONTRAST_BRIGHTNESS)
{
	contrast = new CVFilterSetting("Contrast", 1.0f);
	brightness = new CVFilterSetting("Brightness", 0);
	settings.Add(2, 
		contrast, brightness);
}
int CVContrastBrightnessFilter::Process(CVPipeline * pipe)
{
	// Convert input appropriately to output o.o	
	cv::Mat & image = pipe->input;
	cv::Mat new_image = cv::Mat::zeros( image.size(), image.type() );

	/// Initialize values
	float alpha = contrast->GetFloat();
	float beta = brightness->GetInt();
	int channels = image.channels();

	/// Do the operation new_image(i,j) = alpha*image(i,j) + beta

	uchar * data = image.data;
	uchar * newImageData = new_image.data;
	for( int y = 0; y < image.rows; y++ )
	{
		for( int x = 0; x < image.cols; x++ )
		{ 
			int pixelIndex = y * image.cols + x;
				
			for( int c = 0; c < channels; c++ )
			{
				uchar * pixelData = &data[pixelIndex * channels + c];
				int value = *pixelData;
				int newValue = ((alpha * (int)value) + beta);
				if (newValue > 255)
					newValue = 255;
				if (newValue < 0)
					newValue = 0;
				newImageData[pixelIndex * channels + c] = newValue;
//				new_image.at<cv::Vec3b>(y,x)[c] =
	//				cv::saturate_cast<uchar>(alpha * (image.at<cv::Vec3b>(y,x)[c] ) + beta );
			}
		}
	}
	new_image.copyTo(pipe->output);
	return CVReturnType::CV_IMAGE;
}

CVRandomImage::CVRandomImage()
	: CVImageFilter(CVFilterID::RANDOM_IMAGE)
{
	resolution = new CVFilterSetting("Resolution", Vector2i(0,0));
	bytesPerChannel = new CVFilterSetting("Bytes per Channel", 1);
	settings.Add(2, 
		resolution, bytesPerChannel);
}

int CVRandomImage::Process(CVPipeline * pipe)
{
	// Create a new image!
	int bpc = bytesPerChannel->GetInt();
	int baseType;
	int matrixType;
	int storageBytesPerChannel;
	int64 max;
	switch(bpc)
	{
		case 1: 
			max = 0xFF;
			baseType = CV_8U; 
			storageBytesPerChannel = 1;
			break;
		case 2: 
			max = 0xFFFF; 
			baseType = CV_16U; 
			storageBytesPerChannel = 2;
			break;
		case 3: 
			max = 0xFFFFFF;
			baseType = CV_32S; 
			storageBytesPerChannel = 4;
			break;
		case 4:	
			max = 0xFFFFFFFF;
			baseType = CV_32S; 
			storageBytesPerChannel = 4;
			break;
		default:
			assert(false && "Require at least 1 channel, max ...");
			baseType = CV_8U;
	}
	int channels = 1;
	matrixType = CV_MAKETYPE(baseType, channels);
	Vector2i size(0,0), res = resolution->GetVec2i();
	if (res.x == 0)
		size.x = pipe->input.cols;
	else
		size.x = res.x;
	if (res.y == 0)
		size.y = pipe->input.rows;
	else
		size.y = res.y;
	cv::Mat newImage(size.y, size.x, matrixType);
	
	// PRint it?
//	std::cout<< newImage;

	cv::Mat mean = cv::Mat::zeros(1, 1, matrixType);
	cv::Mat sigma= cv::Mat(1, 1, matrixType, cv::Scalar(max));
	int seed = 0;
	cv::RNG rng(seed);

	// Randomzie contents.

	int dataSize = size.x * size.y * storageBytesPerChannel;
	for (int i = 0; i < dataSize; ++i)
	{
		newImage.data[i] = rand.Randi(255);
	}

//	rng.fill(newImage, cv::RNG::NORMAL, mean, sigma);

	// PRint it?
//	std::cout<<"\n"<<newImage;

	newImage.copyTo(pipe->output);

	return CVReturnType::CV_IMAGE;
}


void RandomizeUChar(cv::Mat & image)
{

}
void RandomizeUShort(cv::Mat & image)
{

}
void RandomizeInt32(cv::Mat & image)
{

}

CVBitFilter::CVBitFilter()
	: CVImageFilter(CVFilterID::BIT_FILTER)
{
	bitsToKeep = new CVFilterSetting("Bits to keep", 4);
	settings.Add(bitsToKeep);
}

int CVBitFilter::Process(CVPipeline * pipe)
{
	if (pipe->input.type() != CV_8UC1)
	{
		errorString = "Image not single channel 8 bit per channel.";
		return -1;
	}

	int filter = 0;
	for (int i = 0; i < bitsToKeep->GetInt(); ++i)
	{
		filter |= (1 << (8 - i - 1));
	}

	pipe->input.copyTo(pipe->output);
	uchar * data = pipe->output.data;
	Vector2i size(pipe->output.cols, pipe->output.rows);
	int pixels = size.x * size.y;
	for (int i = 0; i < pixels; ++i)
	{ 
		data[i] = data[i] & filter;
	}
	return CVReturnType::CV_IMAGE;
}

CVHorizontalDewarp::CVHorizontalDewarp()
	: CVImageFilter(CVFilterID::HORIZONTAL_DEWARP)
{
	center = new CVFilterSetting("Center", 0.5f);
	distance = new CVFilterSetting("Distance", 200.f);
	curvature = new CVFilterSetting("Curvature", 0.5f);
	settings.Add(3, center, distance, curvature);
}

int CVHorizontalDewarp::Process(CVPipeline * pipe)
{
	// Demand single-channel greyscale.
	if (pipe->input.type() != CV_8UC1)
		return CVReturnType::CV_ERROR;
	// Create output based on input.
	uchar * input = pipe->input.data;

	// Create output of same size.
	int rows = pipe->input.rows,
		cols = pipe->input.cols,
		rowStep = pipe->input.step;
	float centerX = cols * center->GetFloat();
	pipe->output = cv::Mat(rows, cols, CV_8UC1);
	uchar * output = pipe->output.data;
	// De-warp it.
	for (int y = 0; y < rows; ++y)
	{
		// Calculate each x separately.
		for (int x = 0; x < cols; ++x)
		{
			int psi = rowStep * y + x;
			// Decide where we should sample from...
			int sampleX, sampleY = y;
			// Calculate where the X sample should be taken from..
			float distToCenter = (x - centerX);
			float absDist = AbsoluteValue(distToCenter);
			float relDist = 1 - absDist / distance->GetFloat();
			ClampFloat(relDist, 0, 1);
			// Now we have some value between 0 and 1 (hopefully).
			// Multiply by 180 degrees (PI) so it is usable as an angle.
			float imaginedAngle = relDist * PI;
			// Run through cosinus (should create a curve from 1 to -1 again)
			float cosine = cos(imaginedAngle) * 0.5f + 0.5f;
			// Use cosine and a multiplier or exponential to apply it as a movement modifier for the sampler?
			float samplerOffset = relDist * 10;
			samplerOffset =	cosine * 10;
			sampleX = x + samplerOffset * (distToCenter < 0? 1 : -1);
			// Clamp it.
			int clampedX = sampleX;
			ClampFloat(sampleX, 0, cols);
			// Get sample and copy it.
			int samplePsi = sampleY * rowStep + clampedX;
			output[psi] = input[samplePsi];
		}
	}


	return CVReturnType::CV_IMAGE;
}


