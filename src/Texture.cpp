/// Emil Hedemalm
/// 2014-02-17
/// General texture class, mostly based on PNG and RGBA types.

#include "Texture.h"
#include <vector>
#include <fstream>
#include "File/FileUtil.h"
#include "LodePNG/lodepng.h"
#include <cassert>
#include "File/LogFile.h"

#include "Libs.h"

#ifdef OPENCV
#include "opencv2/opencv.hpp"
#endif

#include "Image/ImageLoaders.h"

#include "Graphics/GraphicsManager.h"

int Texture::IDenumerator = 0;

/** If true, will make texture memory in CPU be cleared after successful bufferization to video memory.
	Textures marked with 'dynamic' boolean will not be cleared in this manner.
	Default is true, as it should reduce RAM consumption considerably.
*/
bool Texture::releaseOnBufferization = true;


Texture::Texture()
{
	// Strings should initialize correctly as it is...
	glid = -1;
	data = 0;
	height = 0;
	width = 0;
	users = 0;
	// Default to 4 bytes per pixel RGBA. one byte per channel.
	bpp = 4;
	format = RGBA;
	id = IDenumerator++;
	dynamic = false;
	mipmappingEnabled = true;
	dataBufferSize = 0;
	samplingMode = LINEAR;
	creationDate = lastUpdate = Timer::GetCurrentTimeMs();

	fData = NULL;
	cData = NULL;
	data = NULL;

	isDepthTexture = false;
	channels = 4;
	bytesPerChannel = 1;
	bufferized = false;
}

Texture::~Texture()
{
	// Deallocate the glid's here!
	if (glid != -1){
		glDeleteTextures(1, &glid);
		glid = -1;
	}
	Deallocate();
}


void Texture::Deallocate()
{
	delete[] data;
	data = NULL;
	cData = NULL;
	fData = NULL;
}

// Reallocate based on new size and format.
void Texture::Reallocate()
{
	if (!data == 0)
	{
		Deallocate();
	}
	assert(data == 0);
	
	// Allocate.
	int areaInPixels = size[0] * size[1];
	int channels = GetChannels();
	int bufferSizeNeeded = areaInPixels * channels;
	int dataType = DataType();

	width = size[0];
	height = size[1];

	// Save new size.
	dataBufferSize = bufferSizeNeeded;

	if (dataType == DataType::FLOAT)
	{
		fData = new float[bufferSizeNeeded];
		memset(fData, 0, sizeof(float));
		data = (unsigned char*)fData;
	}
	else if (dataType == DataType::UNSIGNED_CHAR)
	{
		cData = new unsigned char[bufferSizeNeeded];
		memset(cData, 0, sizeof(unsigned char));
		data = cData;
	}
}

// Same thing as Resize.
void Texture::SetSize(Vector2i newSize)
{
	Resize(newSize);
}

/// Resets width, height and creates a new data buffer after deleting the old one. Returns false if it failed (due to lacking memory).
bool Texture::Resize(Vector2i newSize)
{
	width = newSize[0];
	height = newSize[1];
	
	/// Always 4-channel data array no matter what part of it will be used!.
	int sizeRequired = newSize[0] * newSize[1] * 4 * bytesPerChannel;
	if (sizeRequired != dataBufferSize)
	{
		if (data)
			delete[] data;
		data = NULL;
		if (!CreateDataBuffer(sizeRequired))
		{
			size = Vector2i(0,0);
			return false;
		}
	}
	size = newSize;
	lastUpdate = Timer::GetCurrentTimeMs();
	return true;
}

/// Loads from file. Can call to reload data even if already loaded once.
bool Texture::LoadFromFile()
{
	// Prefer OpenCV since it's faster.
	bool ok;
	ok = LoadOpenCV(source, this);
	if (!ok)
	{
		std::cout<<"\nOpenCV failed to read texture D:";
	}
	if (!ok)
		ok = LoadPNG(source, this);
	if (!ok)
		ok = LoadLodePNG(source, this);
	if (!ok)
	{
		return false;
	}
	bufferized = false; // Flag that bufferization may by needed?
	// Set it as newly loaded if successful.
	return ok;
}

// Flips along Y axis?
void Texture::FlipY()
{
	// RGBA ftw
	if (bpp != 4)
		return;
	// Row Buffer! o-o
	static unsigned char buf[2048 * 4];
	int lineBufSize = width * bpp;
	for (int y = 0; y < height * 0.5f; y++)
	{
		memcpy(buf, &data[y * lineBufSize], lineBufSize);
		memcpy(&data[y * lineBufSize], &data[(height - y - 1) * lineBufSize], lineBufSize);
		memcpy(&data[(height - y - 1) * lineBufSize], buf, lineBufSize);
	}
}

// Flips along both X and Y axis.
void Texture::FlipXY()
{
	// RGBA ftw
	if (bpp != 4)
		return;
	unsigned char r,g,b,a;
	for (int i = 0; i < dataBufferSize * 0.5f; i+=4)
	{
		r = data[i];
		g = data[i+1];
		b = data[i+2];
		a = data[i+3];
		data[i] = data[dataBufferSize - i - 4];
		data[i+1] = data[dataBufferSize - i - 3];
		data[i+2] = data[dataBufferSize - i - 2];
		data[i+3] = data[dataBufferSize - i - 1];
		data[dataBufferSize - i - 4] = r;
		data[dataBufferSize - i - 3] = g;
		data[dataBufferSize - i - 2] = b;
		data[dataBufferSize - i - 1] = a;
	}
/*	for (int y = 0; y < height; ++y)
	{
		int yStep = y * width * bpp;
		for (int x = 0; x < width; ++x)
		{
			int xStep = x * bpp;
			int index =  yStep + xStep;
			data[] 
		}		
	}
	*/

}

/// Creates the data buffer. Width, height and bpp must be set before hand.
bool Texture::CreateDataBuffer(int withGivenSize /*= -1*/)
{
	if (data)
		return false;
	int targetSize = 0;
	if (withGivenSize > 0)
		targetSize = withGivenSize;
	else
		targetSize = width * height * bpp;
	try 
	{
		data = new unsigned char [targetSize];
		memset(data, 0, targetSize);
		dataBufferSize = targetSize;
	} catch (...)
	{
		std::cout<<"\nAllocation failed.";
		return false;
	}
	return true;
}

/// Retrieves a sample color from the texture, using given amount of samples.
Vector4f Texture::GetSampleColor(int samples /*= 4*/)
{
	Vector4f colorTotal;
	int xDist, yDist;
	xDist = yDist = (int)RoundFloat(sqrt((float)samples));
	int stepsPerX = width / xDist;
	int stepsPerY = height / yDist;
	for (int i = 0; i < samples; ++i)
	{
		int xStep = i % xDist;
		int yStep = i / yDist;
		int x = (int) stepsPerX * (xStep + 0.5f);
		int y = (int) stepsPerY * (yStep + 0.5f);
		colorTotal += GetPixel(x,y);
	}
	Vector4f colorAverage = colorTotal / (float)samples;
	return colorAverage;
}

/// 0 - Unsigned char, 1 - Int, 2 - Float
int Texture::DataType()
{
	switch(format)
	{
		case SINGLE_16F:
		case SINGLE_32F:
		case RGB_16F:
		case RGB_32F:
			return DataType::FLOAT;
		case RGB:
		case RGBA:
			return DataType::UNSIGNED_CHAR;
		default:
			assert(false);
	}
	return -1;
}

int Texture::NumPixels()
{
	return size[0] * size[1];
}
	

/// Returns amount of channels, depending on the format.
int Texture::GetChannels()
{
	switch(format)
	{
		case SINGLE_16F:
		case SINGLE_32F:
			return 1;
		case RGB:
		case RGB_16F:
		case RGB_32F:
			return 3;
		case RGBA:
			return 4;
		default:
			assert(false);
	}
	return 0;
}


/// Uses glGetTexImage to procure image data from GL and insert it into the data-array of the texture object.
void Texture::LoadDataFromGL()
{
//	std::cout<<"\nTrying to load data from GL...";
	glBindTexture(GL_TEXTURE_2D, glid);
	CheckGLError("Texture::LoadDataFromGL - Bind");
	

	// Calculate 
	Reallocate();

	int loadChannels = 0;

	switch(format)
	{
		case RGB:
		//	glReadPixels(0,0,size[0], size[1], GL_RGB, GL_UNSIGNED_BYTE, cData);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, cData); 
			loadChannels = 3;
			break;
		case RGBA:
		{
			loadChannels = 4;
			// glReadPixels might be safer..
		//	glReadPixels(0,0,size[0], size[1], GL_RGBA, GL_UNSIGNED_BYTE, cData);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, cData); 
			break;
		}
		case SINGLE_32F:
		case SINGLE_16F:
			loadChannels = 1;
			//glReadPixels(0,0,size[0], size[1], GL_RED, GL_FLOAT, fData);
			// Try the various formats available until one succeeds?
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, fData); 
			if (glGetError() != GL_NO_ERROR)
			{
				isDepthTexture = true;
				glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, fData);
			}
//			CheckGLError("Texture::LoadDataFromGL - glGetTexImage");
			// Print data.
			/*
			float previous;
			for (int i = 0; i < dataBufferSize ; ++i)
			{
				if (fData[i] != previous)
				{
					previous = fData[i];
					std::cout<<"\n"<<i<<": "<<fData[i];
				}
			}
			*/
			break;
		case RGB_16F:
		case RGB_32F:
		{
			loadChannels = 3;
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, fData); 
			break;
		}
		default:
			assert(false);
	}
	int error = CheckGLError("Texture::LoadDataFromGL");
	if (error == GL_NO_ERROR)
	{
		channels = loadChannels;
	}
	assert(channels > 0);
	// If successful, copy it?
}

/// Loads data from target OpenCV mat.
void Texture::LoadFromCVMat(cv::Mat & mat)
{
#ifdef OPENCV
	// Update amount of channels and bytes per pixel/channel.
	bytesPerChannel = 1;
	/// Depending on the depth, parse differently below.
	int channelDepth = mat.depth();
	switch(channelDepth)
	{
		case CV_8U: case CV_8S: 
			bytesPerChannel = 1;
			break;
		case CV_16U: case CV_16S: 
			bytesPerChannel = 2;
			break;
		case CV_32S: case CV_32F: 
			bytesPerChannel = 4;
			break;
		default:
			assert(false);
			return;
	}
	channels = mat.channels();
	this->bpp = bytesPerChannel * channels;
	Vector2i newSize(mat.cols, mat.rows);
	/// First update size of our texture depending on the given one.
	Resize(newSize);
	
//	std::cout<<"\nMat step: "<<mat.step;
	
	// Just copy the data..?
	int type = mat.type();
	switch(type)
	{
		case CV_8UC3:
		{
			// cv::Mat accessors
			uchar * matData = mat.data;
			// Own matrix accessors
			unsigned char * buf = this->data;
			for (int y = 0; y < mat.rows; ++y)
			{
				int rowOffset = (mat.step * y);
				for (int x = 0; x < mat.cols; ++x)
				{
					/// Pixel start index.
					int psi = rowOffset + x * 3;
					/// Depending on the step count...
					unsigned char b, g, r;
					b = matData[psi];
					g = matData[psi+1];
					r = matData[psi+2];
		//			SetPixel(x, mat.rows - y - 1, Vector4f(r / 255.f,g / 255.f,b / 255.f,1));
					#define ONE_DIV_255 0.00392156862f
					Vector4f color = Vector4f(r * ONE_DIV_255, g * ONE_DIV_255, b * ONE_DIV_255,1);
					color.Clamp(0, 1);
					int texY = mat.rows - y - 1;
					psi = (texY * width + x) * 4;
					buf[psi] = (unsigned char) (color[0] * 255.0f);
					buf[psi+1] = (unsigned char) (color[1] * 255.0f);
					buf[psi+2] = (unsigned char) (color[2] * 255.0f);
					buf[psi+3] = 255;
				}
			}
			break;
		}
		case CV_8UC4:
			memcpy(this->data, mat.data, dataBufferSize);
			break;

		// All other types..?
		case CV_8UC1:
		{
			// cv::Mat accessors
			uchar * matData = mat.data;
			// Own matrix accessors
			unsigned char * buf = this->data;
			for (int y = 0; y < mat.rows; ++y)
			{
				int rowOffset = (mat.step * y);
				for (int x = 0; x < mat.cols; ++x)
				{
					/// Pixel start index.
					int psi = rowOffset + x;
					/// Depending on the step count...
					unsigned char b, g, r;
					b = g = r = matData[psi];
		//			SetPixel(x, mat.rows - y - 1, Vector4f(r / 255.f,g / 255.f,b / 255.f,1));
					#define ONE_DIV_255 0.00392156862f
					Vector4f color = Vector4f(r * ONE_DIV_255, g * ONE_DIV_255, b * ONE_DIV_255,1);
					color.Clamp(0, 1);
					int texY = mat.rows - y - 1;
					psi = (texY * width + x) * 4;
					buf[psi] = (unsigned char) (color[0] * 255.0f);
					buf[psi+1] = (unsigned char) (color[1] * 255.0f);
					buf[psi+2] = (unsigned char) (color[2] * 255.0f);
					buf[psi+3] = 255;
				}
			}
			break;
		}
		case CV_16UC1:
		{
			// cv::Mat accessors
			int size = sizeof(ushort);
			ushort * matShortData = (ushort*)mat.data;
			// Own matrix accessors
			unsigned char * buf = this->data;
			ushort * shortBuf = (ushort*) this->data;
			for (int y = 0; y < mat.rows; ++y)
			{
				int yOffset = y * mat.cols;
				for (int x = 0; x < mat.cols; ++x)
				{
					/// Pixel start index.
					int psi = yOffset + x;
					ushort r, g, b;
					r = g = b = matShortData[psi];
					// Recalculate psi for our own data which is forced to RGBA format.
					int texY = mat.rows - y - 1;
					psi = (texY * width + x) * 4;
					shortBuf[psi] = r;
					shortBuf[psi+1] = g;
					shortBuf[psi+2] = b;
					shortBuf[psi+3] = USHRT_MAX;
				}
			}
			break;
		}
		case CV_32F:
		{
			// Floating point-data.. o.o
			std::cout<<"\ncv::Mat in 32-bit floating point format. Might want to process it further?";
			break;	
		}
		default:
			assert(false && "Implement");
	}

	/// Mark it as dynamic so buffering works properly... <- Must be old.
//	dynamic = true;

	/// Send a message so that the texture is re-buffered.
	Graphics.QueueMessage(new GMBufferTexture(this));

#endif
}


/// For debugging.
bool Texture::MakeRed(){
	for (int h = 0; h < height; ++h){
		for (int w = 0; w < width; ++w){
			/// Get pixel start index.
			int psi = h * width + w;
			data[psi] = 255;
			data[psi] = 0;
			data[psi] = 0;
			data[psi] = 255;
		}
	} 
	return true;
}


/// Bufferizes into GL. Should only be called from the render-thread!
bool Texture::Bufferize(bool force /* = false*/)
{	
	assert(source.Length());
	assert(name.Length());
	/// Don't bufferize multiple times if not special texture, pew!
	if (glid != -1 && !dynamic && !force){
//		std::cout<<"\nTexture \""<<source<<"\" already bufferized! Skipping.";
		return false;
	}
	if (height == 0 || width == 0)
	{
		// nothing to buferize..
		return false;
	}

	assert(data);
	if (!data)
	{
		std::cout<<"\nNo data to bufferize!";
		return false;
	}
	LogGraphics("Buffering texture "+source, INFO);
	queueRebufferization = false;

	GLuint error;
	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float z = -4;
	// Buffer it again..
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, overlayTexture->data.width, overlayTexture->data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, overlayTexture->data.data);
	error = glGetError();
	if (error != GL_NO_ERROR){
		std::cout<<"\nGLError in Render "<<error;
	}
	if (!dynamic)
		std::cout<<"\nBuffering texture "<<name<<"...";
	// Generate texture
	if (glid == -1)
		glGenTextures(1, &glid);

	// Set texturing parameters
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	// Enable texturing
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, glid);  // Bind glTexture ID.

	// Set mip-mapping and such properties
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// Configure sampling outside (0,0), (1,1). GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER available, https://open.gl/textures
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	// Generate the texture Entity in GL
	// Ref: http://www.opengl.org/sdk/docs/man/xhtml/glTexImage2D[0]ml
	/// Pretty much bits or bytes per pixel/channel.
	int pixelDataType;
	int glFormat;
	switch(format)
	{
		case GREYSCALE:	glFormat = GL_RED; break;
		case RGB:		glFormat = GL_RGB; break;
		case RGBA:		glFormat = GL_RGBA;break;
		default:
			assert(false);
	}
	glFormat = GL_RGBA;
	switch(bytesPerChannel)
	{
		case 1:	pixelDataType = GL_UNSIGNED_BYTE; break;
		case 2:	pixelDataType = GL_UNSIGNED_SHORT; break;
		case 3:
		case 4:	pixelDataType = GL_UNSIGNED_INT; break;
		default:
			assert(false);
	}
	
	glTexImage2D(GL_TEXTURE_2D, 0, glFormat, width, height, 0, glFormat, pixelDataType, data);
	CheckGLError("Bufferize");
	
	/// Generate mip-maps!
	if (mipmappingEnabled){
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	/// Or not, generate single texture.
	else {
		/// wat.	
	}
	error = glGetError();
	if (error != GL_NO_ERROR){
		std::cout<<"\nGL Error in TextureManager::BufferizeTexture: "<<error;
	}
	queueRebufferization = false;

	/// Release the kraken! o.o
	if (releaseOnBufferization && !dynamic)
	{
		std::cout<<"\nTexture "<<name<<" buffered. Deleting allocated memory from loading procedure.";
		delete[] data;
		data = NULL;
		dataBufferSize = 0;
		/// Release cv-memory too, if possible?
	}
	/// Notice again.
	if (!dynamic)
		std::cout<<"\nTexture "<<name<<" bufferized.";
	bufferized = true;
}

void Texture::SetSource(String str)
{
	source = str;
	if (name.Length() == 0)
		name = source;
}
void Texture::SetName(String str){
	name = str;
}


/// Gets pixel from indice.
Vector4f Texture::GetPixel(int index){
	assert(bpp == 4 && format == Texture::RGBA);
	Vector4f color;
	unsigned char * buf = data;
	/// PixelStartIndex
	int psi = index * bpp;
	color[0] = buf[psi] / 255.0f;
	color[1] = buf[psi+1] / 255.0f;
	color[2] = buf[psi+2] / 255.0f;
	color[3] = buf[psi+3] / 255.0f;
	return color;
}

/// Gets color data from specified pixel in RGBA
Vector4f Texture::GetPixel(int x, int y){
	assert(bpp == 4 && format == Texture::RGBA);
	Vector4f color;
	unsigned char * buf = data;
	/// PixelStartIndex
	int psi = y * width * bpp + x * bpp;
	color[0] = buf[psi] / 255.0f;
	color[1] = buf[psi+1] / 255.0f;
	color[2] = buf[psi+2] / 255.0f;
	color[3] = buf[psi+3] / 255.0f;
	return color;
}

/// Gets color data from specified pixel in RGBA
Vector4i Texture::GetPixelVec4i(int x, int y)
{
	assert(bpp == 4 && format == Texture::RGBA);
	Vector4i color;
	unsigned char * buf = data;
	/// PixelStartIndex
	int psi = y * width * bpp + x * bpp;
	color[0] = buf[psi];
	color[1] = buf[psi+1];
	color[2] = buf[psi+2];
	color[3] = buf[psi+3];
	return color;
}

/// Samples color from given location, using a weighted average from the neighbouring pixels, based on X and Y co-ordinates.
Vector4f Texture::Sample(float x, float y)
{
	float remainderX = x - (int)x, 
		remainderY = y - (int)y;
	Vector4f base = GetPixel(x,y),
		plusX = GetPixel(x+1,y),
		plusY = GetPixel(x,y+1),
		plusXY = GetPixel(x+1,y+1);

	float ratioBaseX = 1 - remainderX,
		ratioBaseY = 1 - remainderY;
	

	Vector4f sample;
	if (ratioBaseX == ratioBaseY && ratioBaseX == 1)
		return base;
	if (ratioBaseX == 1.f)
	{
		// Just sampel Y..
		assert(false && "Implement");
	}
	else if (ratioBaseY == 1.f)
	{
		// Sample in X.
		sample = base * ratioBaseX + plusX * remainderX;
	}
	/// Quad-sampling...
	else 
	{
		assert(false && "implement");
	}
	return sample;
}

/// Sets color of target pixel. 
void Texture::SetPixel(Vector2i location, const Vector4f & toColor, int pixelSize)
{
	assert(data);
	assert(bpp >= 3 && format == Texture::RGBA);
	unsigned char * buf = data;
	Vector4f color = toColor;
	color.Clamp(0, 1);
	/// PixelStartIndex
	
	for (int y = location[1] - pixelSize + 1; y < location[1] + pixelSize; ++y)
	{
		if (y < 0 || y >= height)
			continue;
		for (int x = location[0] - pixelSize + 1; x < location[0] + pixelSize; ++x)
		{
			if (x < 0 || x >= width)
				continue;
			int psi = y * width * bpp + x * bpp;
			buf[psi] = (unsigned char) (color[0] * 255.0f);
			buf[psi+1] = (unsigned char) (color[1] * 255.0f);
			buf[psi+2] = (unsigned char) (color[2] * 255.0f);
			if (bpp > 3)
				buf[psi+3] = (unsigned char) (color[3] * 255.0f);
		}
	}
	lastUpdate = Timer::GetCurrentTimeMs();
}


/// Sets color of target pixel. Returns false if it is out of bounds.
void Texture::SetPixel(int x, int y, const Vector4f & toColor)
{
	Vector4f color = toColor;
	assert(data);
	assert(bpp >= 3 && format == Texture::RGBA);
	unsigned char * buf = data;
	color.Clamp(0, 1);
	/// PixelStartIndex
	int psi = y * width * bpp + x * bpp;
	buf[psi] = (unsigned char) (color[0] * 255.0f);
	buf[psi+1] = (unsigned char) (color[1] * 255.0f);
	buf[psi+2] = (unsigned char) (color[2] * 255.0f);
	if (bpp > 3)
		buf[psi+3] = (unsigned char) (color[3] * 255.0f);
	lastUpdate = Timer::GetCurrentTimeMs();
}

/// Pretty much highest result of: Vector3f(r,g,b).MaxPart() * a   -> perceived intensity on black background.
float Texture::GetMaxIntensity()
{
	assert(bpp == 4 && format == Texture::RGBA);
	float maxIntensity = 0;
	unsigned char * buf = data;
	for (int y = 0; y < height; ++y){
		for (int x = 0; x < width; ++x){
			int psi = y * width * bpp + x * bpp;
			Vector3f color(
				data[psi] / 255.0f,
				data[psi+1] / 255.0f,
				data[psi+2] / 255.0f
			);
			float intensity = color.MaxPart();
			intensity *= data[psi+3] / 255.0f;
			if (intensity > maxIntensity)
				maxIntensity = intensity;
		}
	}
	return maxIntensity;
}

/// Linear addition of all rgb-compontents.
void Texture::Add(ConstVec3fr color, float alpha /*= 0.0f*/){
	assert(bpp == 4 && format == Texture::RGBA);
	unsigned char * buf = data;
	for (int y = 0; y < height; ++y){
		for (int x = 0; x < width; ++x){
			int psi = y * width * bpp + x * bpp;
			buf[psi] += (unsigned char) (color[0] * 255.0f);
			buf[psi+1] += (unsigned char) (color[1] * 255.0f);
			buf[psi+2] += (unsigned char) (color[2] * 255.0f);
			buf[psi+3] += (unsigned char) (alpha * 255.0f);
		}
	}
}

/// Sets color for all pixels, not touching the alpha.
void Texture::Colorize(ConstVec3fr color){
	assert(bpp == 4 && format == Texture::RGBA);
	unsigned char * buf = data;
	for (int y = 0; y < height; ++y){
		for (int x = 0; x < width; ++x){
			int psi = y * width * bpp + x * bpp;
			buf[psi] = (unsigned char) (color[0] * 255.0f);
			buf[psi+1] = (unsigned char) (color[1] * 255.0f);
			buf[psi+2] = (unsigned char) (color[2] * 255.0f);
		}
	}
}

/// Setts color of all pixels.
void Texture::SetColor(const Vector4f & color)
{
	assert(bpp == 4 && format == Texture::RGBA);
	unsigned char * buf = data;
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			int psi = y * width * bpp + x * bpp;
			buf[psi] = (unsigned char) (color[0] * 255.f);
			buf[psi+1] = (unsigned char) (color[1] * 255.f);
			buf[psi+2] = (unsigned char) (color[2] * 255.f);
			buf[psi+3] = (unsigned char) (color[3] * 255.f);
		}
	}
}

void Texture::SetColorOfColumn(int column, const Vector4f & color)
{
	assert(bpp == 4 && format == Texture::RGBA);
	unsigned char * buf = data;
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			if (x != column)
				continue;
			int psi = y * width * bpp + x * bpp;
			buf[psi] = (unsigned char) (color[0] * 255.f);
			buf[psi+1] = (unsigned char) (color[1] * 255.f);
			buf[psi+2] = (unsigned char) (color[2] * 255.f);
			buf[psi+3] = (unsigned char) (color[3] * 255.f);
		}
	}
}
void Texture::SetColorOfRow(int row, const Vector4f & color)
{
	assert(bpp == 4 && format == Texture::RGBA);
	unsigned char * buf = data;
	for (int y = 0; y < height; ++y)
	{
		if (y != row)
			continue;
		for (int x = 0; x < width; ++x)
		{
			int psi = y * width * bpp + x * bpp;
			buf[psi] = (unsigned char) (color[0] * 255.f);
			buf[psi+1] = (unsigned char) (color[1] * 255.f);
			buf[psi+2] = (unsigned char) (color[2] * 255.f);
			buf[psi+3] = (unsigned char) (color[3] * 255.f);
		}
	}
}


/// Sets sampling mode based on samplingMode current value.
void Texture::SetSamplingMode()
{
	switch(samplingMode)
	{
		case LINEAR:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;
		case NEAREST:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			break;
	}
}


/// Saves the texture in it's entirety to target file. If overwrite is false it will fail if the file already exists.
bool Texture::Save(String toFile, bool overwrite /* = false */)
{

	if (!data)
	{
		std::cout<<"\nTexture lacking data to write!";
		return false;
	}
	if (!toFile.Contains(".png"))
		toFile += ".png";
	if (FileExists(toFile) && !overwrite)
		return false;

	return SaveOpenCV(toFile);
	

	/*
	int channels = GetChannels();
	int pixels = NumPixels();
	for (int i = 0; i < pixels; i += 100)
	{
		int ppi = i * channels;
		unsigned char alpha = cData[ppi+3];
		if (alpha == 0)
			continue;
		std::cout<<"\nPixel "<<i<<": "<<(int)cData[ppi];
	}*/


	/// Wow much progress
	std::cout<<"\nSaving texture to file \""<<toFile.c_str()<<"\"...";
	std::vector<unsigned char> image;
	
	LodePNG::Encoder encoder;
	// Set settings for quicker saving... so slow right now...
	LodePNG_EncodeSettings settings = encoder.getSettings();
	// Zlibsettings seems most relevant..
	settings.auto_choose_color = 0;
	settings.zlibsettings.windowSize = 2048;
	settings.zlibsettings.btype = 2;
	settings.zlibsettings.useLZ77 = 0;
	encoder.setSettings(settings);
	encoder.encode(image, this->data, width, height); //decode the png

	try {
		LodePNG::saveFile(image, toFile.c_str()); //load the image file with given filename
	} catch(...)
	{
		std::cout<<"\nError saving file "<<toFile.c_str()<<"!";
		return false;
	}

	return true;
}


/// Returns the relative path of this texture's source.
String Texture::RelativePath() const {
	FilePath path(source);
	return path.RelativePath();
}

bool Texture::SaveOpenCV(String toPath)
{
#ifdef OPENCV
	cv::Mat mat;
	int cvMatChannels = channels;
	int cvFormat = CV_8UC(cvMatChannels);
	mat.create(cv::Size(width, height), cvFormat);
	int bytesPerChannel = 3;
	int non1s = 0;
	List<float> valueTypes;
	for (int y = 0; y < mat.rows; ++y)
	{
		for (int x = 0; x < mat.cols; ++x)
		{
			/// Pixel start index.
			int psiMat = (mat.step * y) + (x * cvMatChannels);
			int psiTex = ((height - y - 1) * width + x) * bpp;
			int psiTex2 = ((height - y - 1) * width + x);
			/// Depending on the step count...
			switch(channels)
			{		
				// Single-channel image.
				case 1:
				{
					// Convert to displayable format?
					if (fData)
					{
						float value = fData[psiTex2];
						if (isDepthTexture)
						{
							// Re-scale it?
							float originalValue = value;
							if (originalValue != 1)
							{
								non1s++;
							}
							if (valueTypes.Size() < 1000)
							{
								if (!valueTypes.Exists(originalValue))
									valueTypes.Add(originalValue);
							}
							float squared = pow(originalValue, 1);
//							float shrunk = value / FLT_MAX;
							// Make it positive?
//							float positivized = shrunk + 1.f;
							value = squared * 255.f;
						}
//						mat.data[psiMat+0] = mat.data[psiMat+1] = mat.data[psiMat+2] = value;
						mat.data[psiMat] = value;
					}
					else if (cData)
						mat.data[psiMat] = cData[psiTex2];
//						mat.data[psiMat+0] = mat.data[psiMat+1] = mat.data[psiMat+2] = cData[psiTex2];
					break;
				}
				/// RGB! or such.
				case 3:
					mat.data[psiMat+0] = data[psiTex+2];
					mat.data[psiMat+1] = data[psiTex+1];
					mat.data[psiMat+2] = data[psiTex+0];
					break;
				/// RGBA
				case 4:
					mat.data[psiMat+0] = data[psiTex+2];
					mat.data[psiMat+1] = data[psiTex+1];
					mat.data[psiMat+2] = data[psiTex+0];
					mat.data[psiMat+3] = data[psiTex+3];
					/// Ignore alpha when saving to texture?
					break;
				// Default gray scale?
				default:
					assert(false);
					break;
			}
		}
	}
	if (non1s > 0)
		std::cout<<"\nNon 1s: "<<non1s;
	if (valueTypes.Size())
	{
		std::cout<<"\n"<<(valueTypes.Size() > 1000? "More than " : "") <<valueTypes.Size()<<" value types found.";
		/*
		for (int i = 0; i < valueTypes.Size(); ++i)
		{
			std::cout<<"\n"<<i<<": "<<valueTypes[i];
		}*/
	}

	// Write it!
	std::vector<int> compression_params;
	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	// Quick save!
	compression_params.push_back(3);
	cv::imwrite(toPath.c_str(), mat, compression_params);
#endif
	return true;
}



/** An own PNG-saving function, since the library I'm using now is a tad slow (compared to OpenCV) while OpenCV demands 
	additional dependencies which may not be wanted!
*/
bool Texture::SavePNG(String toPath)
{
	std::fstream file;
	file.open(toPath.c_str(), std::ios_base::out);
	if (!file.is_open())
		return false;





	file.close();
	return true;
}

