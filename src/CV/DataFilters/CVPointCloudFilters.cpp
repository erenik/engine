/// Emil Hedemalm
/// 2014-08-14
/// Point cloud specific filters.

#include "CVDataFilters.h"
#include "CV/CVPipeline.h"

#include "CV/Data/CVPointCloud.h"

CVPointClouds::CVPointClouds()
	: CVDataFilter(CVFilterID::POINT_CLOUDS)
{
	input = new CVFilterSetting("Input", 0);
	divisionScheme = new CVFilterSetting("Division scheme", 0);
	settings.Add(2, input, divisionScheme);
	about = "Input: 0 - Corners (e.g. ShiTomasi)\n1 - Optical flow points\nDivision: 0 - Entire screen,\n1 - Binary division horizontally";
}

int CVPointClouds::Process(CVPipeline * pipe)
{
	pipe->pointClouds.Clear();

	// Gather points
	List<Vector2f> points;
	switch(input->GetInt())
	{
		// Corners
		case 0:
		{
			for (int i = 0; i < pipe->corners.size(); ++i)
			{
				cv::Point2f & cvPoint = pipe->corners[i];
				Vector2f point = Vector2f(cvPoint.x, cvPoint.y);
				point *= pipe->currentScaleInv;
				points.Add(point);
			}
			break;
		}
		// Optical flow points
		case 1:
		{
			// Get points from optical flow.
			List<OpticalFlowPoint> & ofp = pipe->opticalFlowPoints;
			for (int i = 0; i < ofp.Size(); ++i)
			{
				Vector2f & vec = ofp[i].previousPosition;
				points.Add(Vector2f(vec.x, vec.y));
//				cv::Point2f cvPoint(vec.x, vec.y);
	//			pts.push_back(cvPoint);
			}
			break;
		}
		default:
			std::cout<<"\nCVPointClouds: Not a valid input.";
			errorString = "CVPointClouds: Not a valid input.";
	}

	// o-o
	switch(divisionScheme->GetInt())
	{
		// Whole screen in one.
		case 0:
		{
			CVPointCloud pointCloud;
			pointCloud.points = points;
			pipe->pointClouds.Add(pointCloud);
			break;
		}
		/// Binary division horizontally
		case 1:
		{
			CVPointCloud left, right;
			Vector2f center = pipe->initialInputSize * 0.5f;
			for (int i = 0; i < points.Size(); ++i)
			{
				Vector2f & point = points[i];
				if (point.x > center.x)
					right.points.Add(point);
				else 
					left.points.Add(point);
			}
			pipe->pointClouds.Add(2, left, right);
			break;
		}
	}
	
	returnType = CVReturnType::POINT_CLOUDS;
	return returnType;
}


/// Principal component analysis
CVPCA::CVPCA()
	: CVDataFilter(CVFilterID::PRINCIPAL_COMPONENT_ANALYSIS)
{
}

CVPCA::~CVPCA()
{
}
int CVPCA::Process(CVPipeline * pipe)
{
	for (int i = 0; i < pipe->pointClouds.Size(); ++i)
	{

		CVPointCloud & pointCloud = pipe->pointClouds[i];
		pointCloud.CreateCVPointsIfNeeded();

		/// Calculate PCA on all available points..
		/// http://robospace.wordpress.com/2013/10/09/object-orientation-principal-component-analysis-opencv/
		std::vector<cv::Point2f> & pts = pointCloud.cvPoints;

		/*
		// Get points from optical flow.
		List<OpticalFlowPoint> & ofp = pipe->opticalFlowPoints;
		for (int i = 0; i < ofp.Size(); ++i)
		{
			Vector2f & vec = ofp[i].previousPosition;
			cv::Point2f cvPoint(vec.x, vec.y);
			pts.push_back(cvPoint);
		}
		*/
		cv::Mat data_pts;
		//Construct a buffer used by the pca analysis
	   data_pts = cv::Mat(pts.size(), 2, CV_64FC1);
		for (int i = 0; i < data_pts.rows; ++i)
		{
			cv::Point2f & pt = pts[i];
			data_pts.at<double>(i, 0) = pt.x;
			data_pts.at<double>(i, 1) = pt.y;
		}
 
		cv::PCA pca_analysis;

		//Store the eigenvalues and eigenvectors
		std::vector<cv::Point2d> eigen_vecs(2);
		std::vector<double> eigen_val(2);
		cv::Point pos;

		//Perform PCA analysis
		try {
			// No use even trying if no points to run it on, yo?
			if (data_pts.rows <= 1)
				continue;
			pca_analysis = cv::PCA(data_pts, cv::Mat(), CV_PCA_DATA_AS_ROW);

			//Store the position of the object
			pos = cv::Point(pca_analysis.mean.at<double>(0, 0), pca_analysis.mean.at<double>(0, 1));
 
			cv::Mat & eigenValues = pca_analysis.eigenvalues;
			for (int i = 0; i < 2; ++i)
			{
				eigen_vecs[i] = cv::Point2d(pca_analysis.eigenvectors.at<double>(i, 0),
										pca_analysis.eigenvectors.at<double>(i, 1));
 
				eigen_val[i] = eigenValues.at<double>(i, 0);
			}

			/// ?
			float orientation = atan2(eigen_vecs[0].y, eigen_vecs[0].x);
		} 
		catch (...)
		{
			errorString = "PCA failed";
			CATCH_EXCEPTION("CVPCA");
			// Skip painting.
			continue;
		}
 
		// Draw the principal components
		pointCloud.pcaCenter = Vector2f(pos.x, pos.y);
		pointCloud.eigenVectors.Clear();
		pointCloud.eigenValues.Clear();
		for (int i = 0; i < eigen_vecs.size(); ++i)
		{
			Vector2f eigenVector;
			eigenVector.x = eigen_vecs[i].x;
			eigenVector.y = eigen_vecs[i].y;
			pointCloud.eigenVectors.Add(eigenVector);
		}
		for (int i = 0; i < eigen_val.size(); ++i)
		{
			float eigenValue = eigen_val[i];
			pointCloud.eigenValues.Add(eigenValue);
		}
	}

	returnType = CVReturnType::POINT_CLOUDS;
	return returnType;
}


CVPointCloudPositionalFilter::CVPointCloudPositionalFilter()
	: CVDataFilter(CVFilterID::POINT_CLOUD_POSITIONAL_FILTER)
{
	filterType = new CVFilterSetting("Filter type", 0);
	param1 = new CVFilterSetting("Param1", 0.0f);
	vectorParam1 = new CVFilterSetting("VecParam1", Vector3f());
	settings.Add(3, filterType, param1, vectorParam1);
	about = "Filter type: 0 - X min\n\
			1 - Discard within vicinity";
}

CVPointCloudPositionalFilter::~CVPointCloudPositionalFilter()
{

}

int CVPointCloudPositionalFilter::Process(CVPipeline * pipe)
{
	float p1 = param1->GetFloat();
	Vector3f vecP1 = vectorParam1->GetVec3f();
	bool remove;
	for (int i = 0; i < pipe->pointClouds.Size(); ++i)
	{
		CVPointCloud & pc = pipe->pointClouds[i];
		for (int j = 0; j < pc.points.Size(); ++j)
		{
			remove = false;
			Vector2f & point = pc.points[j];
			switch(filterType->GetInt())
			{
				// X min
				case 0:
					if (point.x > p1)
					{
						remove = true;
					}
					break;
				case 1:
				{
					float dist = (vecP1 - point).Length();
					if (dist < p1)
					{
						remove = true;	
					}
					break;
				}
				default:
					std::cout<<"\nUndefined filter type.";
			}
			// If remove was queried..
			if (remove)
			{
				pc.points.RemoveIndex(j);
				--j;
				continue;
			}
		}
	}
	return CVReturnType::POINT_CLOUDS;
}


CVHandsFromPointClouds::CVHandsFromPointClouds()
	: CVDataFilter(CVFilterID::HANDS_FROM_POINT_CLOUDS)
{
	// CVFilterSetting * minimumPoints, * centerType;
	minimumPoints = new CVFilterSetting("Minimum points", 3);
	centerType = new CVFilterSetting("Center type", 0);
	settings.Add(2, minimumPoints, centerType);
	about = "Center type: 0 - inherit PCA result";
}

int CVHandsFromPointClouds::Process(CVPipeline * pipe)
{
	// Clear old hands.
	pipe->hands.Clear();
	for (int i = 0; i < pipe->pointClouds.Size(); ++i)
	{
		CVPointCloud & cloud = pipe->pointClouds[i];
//		std::cout<<"\nCloud "<<i<<" points: "<<cloud.points.Size();
		if (cloud.points.Size() < minimumPoints->GetInt())
		{
	//		std::cout<<" Skipping.";
			pipe->pointClouds.RemoveIndex(i);
			--i;
			continue;
		}
		CVHand newHand;
		switch(centerType->GetInt())
		{
			case 0:
				newHand.center = cloud.pcaCenter;
				break;
			case 1: 
				for (int j = 0; j < cloud.points.Size(); ++j)
				{
					newHand.center += cloud.points[j];
				}
				newHand.center /= cloud.points.Size();
				break;
			default: 
			{
				std::cout<<"\nInvalid center parameter";	
			}
		}
		pipe->hands.Add(newHand);
	}
	return CVReturnType::HANDS;
}



