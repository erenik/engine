/// Emil Hedemalm
/// 2014-04-09
/// Filters that have images as (both input and) output.

#include "CVImageFilters.h"
#include "CVPipeline.h"
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
	cv::cvtColor(pipe->input, pipe->output, CV_BGR2GRAY);
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
	cv::pyrUp(pipe->input, pipe->output, cv::Size(pipe->input.cols * scale->fValue, pipe->input.rows * scale->fValue));
#else
	assert(false && "Either fix includes in linux version or compile pyrUp");
#endif
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
		cv::pyrDown(pipe->input, pipe->output, cv::Size(pipe->input.cols * scale->fValue, pipe->input.rows * scale->fValue));
#else
		assert(false && "Either fix includes in linux version or compile pyrUp");
#endif
	}catch(...)
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
	
	int kernel_size = kernelSize->iValue;
	
	// Always add the 1-bit to the kernel_size, making sure that it's odd.
	// Check that kernel size is odd.
	if (!(kernel_size & 1))
	{
		errorString = "Kernel size must be odd!";
		return CVReturnType::NO_OUTPUT;
	}

	// Adjustments as needed.
	if (edgeThreshold->fValue > max_lowThreshold)
		edgeThreshold->fValue = max_lowThreshold;
	
//	cv::Mat detected_edges;
	try {
		cv::Canny(pipe->input, pipe->output, edgeThreshold->fValue, edgeThreshold->fValue * ratio->fValue, kernel_size);
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
#define UCHAR	1
#define FLOAT	2
		int dataType = 0;
		switch(type)
		{
			case CV_32FC1:
			case CV_32FC2:
			case CV_32FC3:
			case CV_32FC4:
				dataType = FLOAT;
				break;
			default:
				dataType = UCHAR;
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
		if (dataType == UCHAR)
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
		else if (dataType == FLOAT)
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
	else if (!(kSize->iValue & 1))
	{
		errorString = "kSize must be odd.";
		return CVReturnType::NOTHING;
	}
	else if (kSize->iValue > 31)
	{
		errorString = "kSize may not go above 31";
		return CVReturnType::NOTHING;
	}
	/// Result is a 32-bit floating point signed image
	cv::Mat detected_corners;
	try {
		cv::cornerHarris(pipe->input, pipe->output, blockSize->iValue, kSize->iValue, k->fValue);
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
	settings.Add(blurSize);
}

int CVGaussianBlurFilter::Process(CVPipeline * pipe)
{
	if (!(blurSize->iValue & 1))
	{
		errorString = "Blur size must be odd.";
		return CVReturnType::NOTHING;
	}
	try {
		cv::GaussianBlur(pipe->input, pipe->output, cv::Size(blurSize->iValue, blurSize->iValue), 0, 0);
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
		cv::threshold(pipe->input, pipe->output, threshold->fValue, maxValue->fValue, type->iValue);
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
#define UCHAR	1
#define FLOAT	2
		int dataType = 0;
		switch(type)
		{
			case CV_32FC1:
			case CV_32FC2:
			case CV_32FC3:
			case CV_32FC4:
				dataType = FLOAT;
				break;
			default:
				dataType = UCHAR;
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
		if (dataType == UCHAR)
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
		else if (dataType == FLOAT)
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
	
	switch(channelToDisplay->iValue)
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
	Range targetRange(targetHue->iValue - range->iValue, targetHue->iValue + range->iValue);
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
			if (value >= targetSaturation->iValue - scope->iValue &&
				value <= targetSaturation->iValue + scope->iValue)
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
//	replacementColor = bnew
	settings.Add(targetValue);
	settings.Add(scope);

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
	{
		errorString = "No available value data.";
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
			if (value >= targetValue->iValue - scope->iValue &&
				value <= targetValue->iValue + scope->iValue)
			{
				// Replace with saturation
				outputData[index] = 0;	
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
		if (automatic->bValue)
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
			if (framesPassed >= framesToConsider->iValue)
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
				if (sumTotal < threshold->fValue){
					extractBackground->bValue = true;
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

	if(extractBackground->bValue)
	{
		pipe->input.copyTo(background);
		errorString = "Background saved from input.";
		extractBackground->bValue = false;
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
	switch(type->iValue)
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
									cv::Size(size->iValue * 2 + 1, size->iValue * 2 + 1),
									cv::Point(size->iValue, size->iValue)
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
	switch(type->iValue)
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
									cv::Size(size->iValue * 2 + 1, size->iValue * 2 + 1),
									cv::Point(size->iValue, size->iValue)
								);
		cv::dilate(pipe->input, pipe->output, erosionElement);
	} catch(...)
	{
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
			for (int pixelX = x; pixelX < columns && pixelX < x + maxDistanceToCheck->iValue; ++pixelX)
			{
				if (cData[yStep + pixelX * channels])
				{
					neighbours++;
					break;
				}
			}
			// Check x-wise.. westward!!
			for (int pixelX = x; pixelX >= 0 && pixelX > x - maxDistanceToCheck->iValue; --pixelX)
			{
				if (cData[yStep + pixelX * channels])
				{
					neighbours++;
					break;
				}
			}
			// Check y-wise.. upwards!!!
			for (int pixelY = y; pixelY >= 0 && pixelY > y - maxDistanceToCheck->iValue; --pixelY)
			{
				// Multiplying channels could be omitted due to the image being single-channel..
				if (cData[pixelY * columns * channels + xStep])
				{
					neighbours++;
					break;
				}
			}

			// Check y-wise.. down o.o
			for (int pixelY = y; pixelY < rows && pixelY < y + maxDistanceToCheck->iValue; ++pixelY)
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

