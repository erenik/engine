/// Emil Hedemalm
/// 2014-08-06
/// Classes and filters pertaining to optical flow.

#include "CV/DataFilters/CVDataFilters.h"
#include "Matrix/Matrix.h"
#include "OpticalFlow.h"

class Entity;

// Optical flow, whole image o-o
class CVOpticalFlowFarneback : public CVDataFilter
{
public:
	CVOpticalFlowFarneback();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * pyramidScale, * pyramidLayers, 
		* averagingWindowSize, * iterations, * polynomialNeighbourhoodSize,
		 * polynomialSmoothingSigma, * outputType;

	cv::Mat lastFrame;
};

class CVOpticalFlowLucasKanade : public CVDataFilter 
{
public:
	CVOpticalFlowLucasKanade();
	virtual ~CVOpticalFlowLucasKanade();
	virtual int Process(CVPipeline * pipe);

private:

	void PaintOFPoints(cv::Mat & mat, CVPipeline * pipe);

	CVFilterSetting * searchWindowSize, * maxLevel, * minEigenThreshold, 
		* maxIterations, * epsilon, * minimumDistance, * maxDistance, 
		* quadrantFilter;
	CVFilterSetting * paintPointSize, * directionVectorThickness;
	// Last frame
	cv::Mat lastFrame;
	// Points from last frame?
	std::vector<cv::Point2f> lastPoints;

	// Vectors placed here since they fail brutally if they are placed within the function for some weird reason...
	std::vector<cv::Point2f> points;
	std::vector<uchar> status;
    std::vector<float> err;

	std::vector<cv::Mat> pastPyramids;
	std::vector<cv::Mat> currentPyramids;


	/// o.o For rendering on da projectoooor!
	cv::Mat outputRender;
	Texture * outputRenderTexture;
	Entity * outputRenderEntity;

	CVFilterSetting * renderOutputAsDedicatedEntity;
};

/** Optical-flow based filter which divies the input image in X-parts (both length and width-wise)
	before calculating optical flow for each image part.
*/
class CVOpticalFlowLKSubdivided : public CVDataFilter 
{
public:
	CVOpticalFlowLKSubdivided();
	virtual int Process(CVPipeline * pipe);

private:
	CVFilterSetting * searchWindowSize, * maxLevel, * minEigenThreshold, 
		* maxIterations, * epsilon, * intensityThreshold,
		 * elementsPerRow;


	// Last frame
	cv::Mat lastFrame;
	// Points from last frame?
	std::vector<cv::Point2f> lastPoints;

	// Vectors placed here since they fail brutally if they are placed within the function for some weird reason...
	std::vector<cv::Point2f> points;
	std::vector<uchar> status;
    std::vector<float> err;

};

class CVOpticalFlowSimpleFlow : public CVDataFilter 
{
public:
	CVOpticalFlowSimpleFlow();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * averagingBlockSize, * maxFlow;
	// Last frame
	cv::Mat lastFrame;
	
};


class CVOpticalFlowDualTVL1 : public CVDataFilter 
{
public:
	CVOpticalFlowDualTVL1();
	~CVOpticalFlowDualTVL1();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * tau, * lambda, * theta, * nscales, * warps, * epsilon, * iterations;
	cv::Ptr<cv::DenseOpticalFlow> dualTVL1;
	cv::Mat lastFrame, flow;

};

// Computes an image-wide flow field for the generated optical flow data.
class CVGenerateOpticalFlowField : public CVDataFilter 
{
public:
	CVGenerateOpticalFlowField();
	virtual int Process(CVPipeline * pipe);
	/** Main painting function. This will be called on the last processed filter in order to get a renderable result.
		Separated from main processing since painting can take an unnecessary long amount of time to complete.
	*/
	virtual void Paint(CVPipeline * pipe);	

private:
	CVFilterSetting * segments, * toPaint;
};
