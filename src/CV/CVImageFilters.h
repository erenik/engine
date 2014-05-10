/// Emil Hedemalm
/// 2014-04-11
/// Filters that have images as (both input and) output.

#include "CVFilter.h"

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
	CVFilterSetting * blurSize;
};


class CVThresholdFilter : public CVImageFilter 
{
public:
	CVThresholdFilter();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * threshold, * maxValue, * type;
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
	//CVFilterSetting * targetSaturation, * scope, * replacementColor;
};

/// Operates on a given band of values, painting those with target color.
class CVValueFilter : public CVImageFilter 
{
public:
	CVValueFilter();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * targetValue, * scope, * replacementColor;
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




