/// Emil Hedemalm
/// 2014-04-11
/// Filters that have images as (both input and) output.

#include "CV/CVFilter.h"
#include "CV/OpenCV.h"

class CVImageFilter : public CVFilter
{
public:
	// Constructor that sets the filter-type to CVFilter::DATA
	CVImageFilter(int type);
	/** Main painting function. This will be called on the last processed filter in order to get a renderable result.
		Separated from main processing since painting can take an unnecessary long amount of time to complete.
	*/
	virtual void Paint(CVPipeline * pipe);
};


// Converts image to single-byte/channel per pixel.
class CVToChar : public CVImageFilter 
{
public:
	CVToChar();
	virtual int Process(CVPipeline * pipe);
};

class CVGreyscaleFilter : public CVImageFilter
{
public:
	CVGreyscaleFilter();
	// Main processing function, sub-class and re-implement this.
	virtual int Process(CVPipeline * pipe);
};

class CVScaleUpFilter : public CVImageFilter
{
public:
	CVScaleUpFilter();
	// Main processing function, sub-class and re-implement this.
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * scale;
};

class CVScaleDownFilter : public CVImageFilter
{
public:
	CVScaleDownFilter();
	// Main processing function, sub-class and re-implement this.
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * scale;
};

class CVCannyEdgeFilter : public CVImageFilter
{
public:
	CVCannyEdgeFilter();
	// Main processing function, sub-class and re-implement this.
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * edgeThreshold, * ratio, * kernelSize;

};

class CVInvertFilter : public CVImageFilter 
{
public:
	CVInvertFilter();
	virtual int Process(CVPipeline * pipe);
private:
};

class CVHarrisCornerFilter : public CVImageFilter 
{
public:
	CVHarrisCornerFilter();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * blockSize, * kSize, * k;
};

class CVGaussianBlurFilter : public CVImageFilter 
{
public:
	CVGaussianBlurFilter();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * blurSize, * blurSizeXY, * type;
};


class CVThresholdFilter : public CVImageFilter 
{
public:
	CVThresholdFilter();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * threshold, * maxValue, * type;
};


/// Filter which uses a threshold value based on observed data within the input image.
class CVAdaptiveThreshold : public CVImageFilter 
{
public:
	CVAdaptiveThreshold();
	virtual int Process(CVPipeline * pipe);
private:
	/// Value from 0.0 to 1.0, where 0 will be mapped to the minimum value within the image and 1.0 will be mapped to the maximum value.
	CVFilterSetting * relativeThreshold, 
		* minimumThreshold, 
		* minimumRange;

};

/// Known in OpenCV simple as adaptiveThreshold, compares pixel value with the values in the vicinity.
class CVPerPixelAdaptiveThreshold : public CVImageFilter 
{
public:
	CVPerPixelAdaptiveThreshold();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * maxValue, * adaptiveMethod, * thresholdType,
		* blockSize, * c;
};

// Calculates absoluate-values on every pixel. Used to display floating-based ones after thresholding.
class CVAbsFilter : public CVImageFilter 
{
public:
	CVAbsFilter();
	virtual int Process(CVPipeline * pipe);
private:
};

class CVColorFilter : public CVImageFilter 
{
public:
	CVColorFilter();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * colorToFilter, * colorToReplace, * mode;
};

/// Extracts Hue, Saturation, Value/Light, etc. into the pipeline, depending on chosen method.
class CVExtractChannels : public CVImageFilter 
{
public:
	CVExtractChannels();
	virtual int Process(CVPipeline * pipe);
	// Custom paint method.
	virtual void Paint(CVPipeline * pipe);
private:
	CVFilterSetting * channelToDisplay;
};

/// Uses extracted hue-channel in the pipeline and filters out only the interesting parts. Resulting image is a single-channel binary image.
class CVHueFilter : public CVImageFilter
{
public:
	CVHueFilter();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * targetHue, * range;
};

class CVSaturationFilter : public CVImageFilter 
{
public:
	CVSaturationFilter();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * targetSaturation, * scope, * replacementColor;
};

/// Operates on a given band of values, painting those with target color.
class CVValueFilter : public CVImageFilter 
{
public:
	CVValueFilter();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * targetValue, * scope, * replacementValue;
};


// Background removal, a.k.a. motion filter?
class CVRemoveBackgroundFilter : public CVImageFilter 
{
public:
	CVRemoveBackgroundFilter();
	virtual int Process(CVPipeline * pipe);
private:
	/// Used for background extraction in order to detect any motion.
	cv::Mat background;
	/// Boolean to set in order for the filter to know when to extract the current frame as known background.
	CVFilterSetting * extractBackground, * automatic, * framesToConsider, * threshold;
	/// Matrix to hold summed/average values for when considering when to automatically extract new background.
	cv::Mat framesTotal, diffMat;
	/// Goes from 0 to framesBetween before evaluating if it should get a new background.
	int framesPassed;
};


class CVErode : public CVImageFilter 
{
public:
	CVErode();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * type, * size;
};

class CVDilate : public CVImageFilter 
{
public:
	CVDilate();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * type, * size;
};


// Extracts Lab equivalents to HSV and places them in 
class CVExtractLABColors : public CVImageFilter 
{
public:
	CVExtractLABColors();
	virtual int Process(CVPipeline * pipe);
private:
};


class CVFill3 : public CVImageFilter 
{
public:
	CVFill3();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * maxDistanceToCheck;
};

//class CVSaturationFilter 

;


/// For debug-rendering the region-of-interest where the projector and camera input overlap.
class CVRenderRegionOfInterest : public CVImageFilter 
{
public:
	CVRenderRegionOfInterest();
	virtual int Process(CVPipeline * pipe);
	/** Main painting function. This will be called on the last processed filter in order to get a renderable result.
		Separated from main processing since painting can take an unnecessary long amount of time to complete.
	*/
	virtual void Paint(CVPipeline * pipe);
	/// Build-in messaging system. Used for advanced applications to communicate with the game engine properly.
	virtual void ProcessMessage(Message * message);

private:
	CVFilterSetting * topLeftCorner, * bottomRightCorner;
};


class CVRegionOfInterestFilter : public CVImageFilter 
{
public:
	CVRegionOfInterestFilter();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * topLeftCorner, * bottomRightCorner, * fillColor, * replaceInitialInput;
};


class CVLoadInitialInput : public CVImageFilter 
{
public:
	CVLoadInitialInput();
	virtual int Process(CVPipeline * pipe);
private:
	// If we want to load e.g. previously greyscaled initial input.
	CVFilterSetting * adjustedInput;
};

// Segmentation or adjustment of image based on detected optical flow.
class CVOpticalFlowFilter : public CVImageFilter 
{
public:
	CVOpticalFlowFilter();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * matrixSize, * increasePerFlow;
};

class CVContrastBrightnessFilter : public CVImageFilter 
{
public:
	CVContrastBrightnessFilter();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * contrast, * brightness;
};

/// Filter to properly convert 2-byte images to single-byte images.
class CVShortToUnsignedByte : public CVImageFilter 
{
public:
	CVShortToUnsignedByte();
	virtual int Process(CVPipeline * pipe);
private:
	// Selects where the most significant byte should start, choosing the next 7 bits automatically.
	CVFilterSetting * mostSignificantBit;
};


#include "Random/Random.h"

/// Used to generate random images of chosen type.
class CVRandomImage : public CVImageFilter 
{
public:
	CVRandomImage();
	virtual int Process(CVPipeline * pipe);
private:
	void RandomizeUChar(cv::Mat & image);
	void RandomizeUShort(cv::Mat & image);
	void RandomizeInt32(cv::Mat & image);

	Random rand;
	/// o.o
	CVFilterSetting * bytesPerChannel;
	/// 0,0 refers to inheriting resolution.
	CVFilterSetting * resolution;
};

class CVBitFilter : public CVImageFilter 
{
public:
	CVBitFilter();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * bitsToKeep;
};

class CVHorizontalDewarp : public CVImageFilter 
{
public:
	CVHorizontalDewarp();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * center, * distance, * curvature;
};

