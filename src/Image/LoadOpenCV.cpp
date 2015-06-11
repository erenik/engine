/// Emil Hedemalm
/// 2015-06-11
/// OpenCV-based PNG (or rather, image) loader.

#include "ImageLoaders.h"

#include "Libs.h"

#ifdef OPENCV
	#include "opencv2/opencv.hpp"
#endif

/// Attempts to load a texture using OpenCV imread.
bool LoadOpenCV(String source, Texture * texture)
{
#ifdef OPENCV
	/// Supported via OpenCV: http://docs.opencv.org/modules/highgui/doc/reading_and_writing_images_and_video.html?highlight=imread#imread
	cv::Mat mat;
	// http://stackoverflow.com/questions/7417637/imread-not-working-in-opencv
	mat = cvLoadImage(source.c_str(), CV_LOAD_IMAGE_UNCHANGED);
//	mat = cv::imread(source.c_str(), CV_LOAD_IMAGE_UNCHANGED);
	if (!mat.cols || !mat.rows)
	{	
		// Bad data? 
		return false;
	}
	/// First update size of our texture depending on the given one.
	if (texture->width != mat.cols || texture->height != mat.rows)
	{
		texture->Resize(Vector2i(mat.cols, mat.rows));
	}
	
//	std::cout<<"\nMat step: "<<mat->step;
	int channels = mat.channels();
	/// Depending on the depth, parse differently below.
	int channelDepth = mat.depth();
	int bytesPerChannel = 1;
	switch(channelDepth)
	{
		case CV_8U: case CV_8S: 
			bytesPerChannel = 1;
			break;
		case CV_16U: case CV_16S: 
			// Convert to single-channel if needed...
			mat.convertTo(mat, CV_8UC3);
			bytesPerChannel = 1;
			break;
		case CV_32S: case CV_32F: 
			bytesPerChannel = 4;
			break;
		default:
			assert(false);
			return false;
	}
	texture->bytesPerChannel = bytesPerChannel;

	/// Fetch data pointer now that any needed conversions are done.
	unsigned char * data = (unsigned char*) (mat.data);
	unsigned char * texData = texture->data;
	float minFloat = 0, maxFloat = 0;	

	for (int y = 0; y < mat.rows; ++y)
	{
		for (int x = 0; x < mat.cols; ++x)
		{
			unsigned char b,g,r,a = 255;
			/// Pixel start index.
			int psi = (mat.step * y) + (x * channels) * bytesPerChannel;
			/// Depending on the step count...
			switch(channels)
			{			
				case 1:
				{
					if (bytesPerChannel == 1)
						b = g = r = data[psi];
					else if (bytesPerChannel == 4)
					{
						float * fPtr = (float*)&data[psi];
						float fValue = *fPtr;
						if (fValue > maxFloat)
							maxFloat = fValue;
						if (fValue < minFloat)
							minFloat = fValue;
						unsigned char cValue = (unsigned char) fValue;
						b = g = r = cValue;
					}
					break;
					
				}
				/// RGB!
				case 3:
					b = data[psi+0];
					g = data[psi+1];
					r = data[psi+2];
					break;
				case 4:
				{
					b = data[psi+0];
					g = data[psi+1];
					r = data[psi+2];
					a = data[psi+3];
					break;
				}
				// Default gray scale?
				default:
					b = g = r = data[psi];
					break;
			}
			// Always rgba!
			int texPsi = ((texture->width * (texture->height - y - 1)) + x) * texture->bpp;
			texData[texPsi+0] = r;
			texData[texPsi+1] = g;
			texData[texPsi+2] = b;
			texData[texPsi+3] = a;
//			texture->SetPixel(x, mat.rows - y - 1, Vector4f(r / 255.f,g / 255.f,b / 255.f,1));
		}
	}
	/// Send a message so that the texture is re-buffered... wat.
//	Graphics.QueueMessage(new GMBufferTexture(texture));
	return true;
#endif
	return false;
}
