/// Emil Hedemalm
/// 2014-02-17
/// General texture class, mostly based on PNG and RGBA types.

#include "Texture.h"
#include <vector>
#include <fstream>
#include "File/FileUtil.h"
#include "LodePNG/lodepng.h"

int Texture::IDenumerator = 0;

Texture::Texture(){
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
	lastUpdate = Timer::GetCurrentTimeMs();
}
Texture::~Texture(){
	// Deallocate the glid's here!
	if (glid != -1){
		glDeleteTextures(1, &glid);
		glid = -1;
	}
	if (data)
		delete[] data;
	data = NULL;
}

/// Resets width, height and creates a new data buffer after deleting the old one.
void Texture::Resize(Vector2i newSize)
{
	width = newSize.x;
	height = newSize.y;
	if (data)
		delete[] data;
	data = NULL;
	CreateDataBuffer();
	lastUpdate = Timer::GetCurrentTimeMs();
}

// Flips along Y axis?
void Texture::FlipY()
{
	// RGBA ftw
	if (bpp != 4)
		return;
	unsigned char r,g,b,a;
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
bool Texture::CreateDataBuffer()
{
	if (data)
		return false;
	dataBufferSize = width * height * bpp;
	data = new unsigned char [dataBufferSize];
	return true;
}

/// Retrieves a sample color from the texture, using given amount of samples.
Vector4f Texture::GetSampleColor(int samples /*= 4*/)
{
	Vector4f colorTotal;
	int xDist, yDist;
	xDist = yDist = RoundFloat(sqrt((float)samples));
	int stepsPerX = width / xDist;
	int stepsPerY = height / yDist;
	for (int i = 0; i < samples; ++i)
	{
		int xStep = i % xDist;
		int yStep = i / yDist;
		int x = stepsPerX * (xStep + 0.5f);
		int y = stepsPerY * (yStep + 0.5f);
		colorTotal += GetPixel(x,y);
	}
	Vector4f colorAverage = colorTotal / samples;
	return colorAverage;
}

/// Uses glGetTexImage to procure image data from GL and insert it into the data-array of the texture object.
void Texture::LoadDataFromGL()
{
	std::cout<<"\nTrying to load data from GL...";
	glBindTexture(GL_TEXTURE_2D, glid);
	unsigned char * tmpArray = new unsigned char[this->dataBufferSize];
	memset(tmpArray, 0, dataBufferSize);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmpArray); 
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
bool Texture::Bufferize()
{	
	queueRebufferization = false;
	/// Don't bufferize multiple times if not special texture, pew!
	if (glid != -1 && !dynamic){
		std::cout<<"\nTexture \""<<source<<"\" already bufferized! Skipping.";
		return false;
	}

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

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	// Enable texturing
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, glid);  // Bind glTexture ID.

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Generate the texture Entity in GL
	// Ref: http://www.opengl.org/sdk/docs/man/xhtml/glTexImage2D.xml
	if (bpp == 4){
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,	width,
				height,		0, 	GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	else if (bpp == 3)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	else {
		assert(false && "Implement D:");
	}

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
}

void Texture::SetSource(String str){
	source = str;
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
	color.x = buf[psi] / 255.0f;
	color.y = buf[psi+1] / 255.0f;
	color.z = buf[psi+2] / 255.0f;
	color.w = buf[psi+3] / 255.0f;
	return color;
}
/// Gets color data from specified pixel in RGBA
Vector4f Texture::GetPixel(int x, int y){
	assert(bpp == 4 && format == Texture::RGBA);
	Vector4f color;
	unsigned char * buf = data;
	/// PixelStartIndex
	int psi = y * width * bpp + x * bpp;
	color.x = buf[psi] / 255.0f;
	color.y = buf[psi+1] / 255.0f;
	color.z = buf[psi+2] / 255.0f;
	color.w = buf[psi+3] / 255.0f;
	return color;
}

/// Sets color of target pixel. Returns false if it is out of bounds.
void Texture::SetPixel(int x, int y, Vector4f color)
{
	assert(data);
	assert(bpp >= 3 && format == Texture::RGBA);
	unsigned char * buf = data;
	color.Clamp(0, 1);
	/// PixelStartIndex
	int psi = y * width * bpp + x * bpp;
	buf[psi] = color.x * 255.0f;
	buf[psi+1] = color.y * 255.0f;
	buf[psi+2] = color.z * 255.0f;
	if (bpp > 3)
		buf[psi+3] = color.w * 255.0f;
	lastUpdate = Timer::GetCurrentTimeMs();
}

/// Pretty much highest result of: Vector3f(r,g,b).MaxPart() * a   -> perceived intensity on black background.
float Texture::GetMaxIntensity(){
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
void Texture::Add(Vector3f color, float alpha /*= 0.0f*/){
	assert(bpp == 4 && format == Texture::RGBA);
	unsigned char * buf = data;
	for (int y = 0; y < height; ++y){
		for (int x = 0; x < width; ++x){
			int psi = y * width * bpp + x * bpp;
			buf[psi] += (unsigned char) (color.x * 255.0f);
			buf[psi+1] += (unsigned char) (color.y * 255.0f);
			buf[psi+2] += (unsigned char) (color.z * 255.0f);
			buf[psi+3] += (unsigned char) (alpha * 255.0f);
		}
	}
}

/// Sets color for all pixels, not touching the alpha.
void Texture::Colorize(Vector3f color){
	assert(bpp == 4 && format == Texture::RGBA);
	unsigned char * buf = data;
	for (int y = 0; y < height; ++y){
		for (int x = 0; x < width; ++x){
			int psi = y * width * bpp + x * bpp;
			buf[psi] = (unsigned char) (color.x * 255.0f);
			buf[psi+1] = (unsigned char) (color.y * 255.0f);
			buf[psi+2] = (unsigned char) (color.z * 255.0f);
		}
	}
}

/// Setts color of all pixels.
void Texture::SetColor(Vector4f color){
	assert(bpp == 4 && format == Texture::RGBA);
	unsigned char * buf = data;
	for (int y = 0; y < height; ++y){
		for (int x = 0; x < width; ++x){
			int psi = y * width * bpp + x * bpp;
			buf[psi] = (unsigned char) (color.x * 255.f);
			buf[psi+1] = (unsigned char) (color.y * 255.f);
			buf[psi+2] = (unsigned char) (color.z * 255.f);
			buf[psi+3] = (unsigned char) (color.w * 255.f);
		}
	}
}

/// Saves the texture in it's entirety to target file. If overwrite is false it will fail if the file already exists.
bool Texture::Save(String toFile, bool overwrite /* = false */)
{
	assert(data);
	if (!toFile.Contains(".png"))
		toFile += ".png";
	if (FileExists(toFile) && !overwrite)
		return false;
	
	/// Wow much progress
	std::cout<<"\nSaving texture to file \""<<toFile.c_str()<<"\"...";
	std::vector<unsigned char> image;

	LodePNG::Encoder encoder;
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