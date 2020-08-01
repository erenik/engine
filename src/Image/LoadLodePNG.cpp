/// Emil Hedemalm
/// 2015-05-30
/// LonePNG PNG loader

#include "ImageLoaders.h"

#include <vector>
#include <fstream>

#include "LodePNG/lodepng.h"

bool LoadLodePNG(String source, Texture * texture)
{
	/// Obsoleted
	std::vector<unsigned char> buffer, image;

	/// Check that the file exists
	try {
		std::ifstream file;
		std::cout<<"\nOpening input-file stream.";
		file.open(source.c_str());
		if (!file.good()){
			// Clear any error flags set!
			file.clear();
			std::cout<<"\nUnable to open file.";
			file.close();
	//		std::cout<<"\nERROR: File "<<source.c_str()<<" does not exist!";
	//		assert("TextureManager::LoadTexture: ERROR: File <<source<< does not exist!");
			return NULL;
		}
		std::cout<<"\nFile exists";
	}
	catch(...)
	{
		std::cout<<"\nErrrrorrrrr"<<std::flush;
		return NULL;
	}
	try {
		LodePNG::loadFile(buffer, source.c_str()); //load the image file with given filename
		std::cout<<"\nFile loaded.";
	} catch(...)
	{
		std::cout<<"\nError loading file "<<source.c_str()<<"!";
	}

	LodePNG::Decoder decoder;
	decoder.decode(image, buffer); //decode the png


	// Check if the decoder got an error
	if(decoder.hasError())
	{	// Move to next texture.
		std::cout<<"\nError decoding "<<source.c_str();
		//MessageBox(NULL, filename, "Error reading texture file", MB_OK | MB_ICONEXCLAMATION);
		return NULL;
	}

	// Set Texture to RGBA since we will assign Alpha-values by default anyway
	texture->SetFormat(Texture::RGBA);

	// Get width, height and pixels
	int width = decoder.getWidth(), height =  decoder.getHeight();
	int bytesPerPixel = decoder.getBpp()/8;

	// Make sure the texture data array isn't already allocated.
	assert(texture->data== NULL);

/// Flipping while loading texture or not...?
#define INDEX_REGULAR	4 * y * width + 4 * x
#define INDEX_REGULAR_S	4 * (y * width + x)
#define INDEX_SWAP_X	4 * y * width + width * 4 - 4 * x - 4
#define INDEX_SWAP_X_S	4 * (y * width + width - x - 1)
#define INDEX_SWAP_Y	width * height * 4 - 4 * y * width + 4 * x - 4 * width
#define INDEX_SWAP_Y_S	4 * (width * height - y * width + x - width)
#define SOURCE_INDEX	INDEX_REGULAR_S
#define TARGET_INDEX	INDEX_SWAP_Y_S

	/// Onry 1 byte per pixchsel?!
	if (bytesPerPixel == 1)
	{
		// Allocate texture data array.
		texture->dataBufferSize = width*height*4;
		texture->data = new unsigned char [texture->dataBufferSize];
		memset(texture->data, '0', texture->dataBufferSize);
		// And save the width and height.
		texture->size = Vector2i(width, height);
		// Go through the whole image.
		for(int j = 0; j < height; j++){
		//	int y = height - j - 1;
			int y = j;
			for(int x = 0; x < width; x++){

				//get RGBA components
				unsigned char r,g,b,a;
				r = g = b = image[SOURCE_INDEX + 0]; //red
				a = 255;
				// Set them into the texture data array.
				texture->data[TARGET_INDEX + 0] = r;
				texture->data[TARGET_INDEX + 1] = g;
				texture->data[TARGET_INDEX + 2] = b;
				texture->data[TARGET_INDEX + 3] = a;
			}
		}

	}
	else if (bytesPerPixel == 4){
		// Allocate texture data array.
		texture->dataBufferSize = width*height*bytesPerPixel;
		texture->data = new unsigned char [texture->dataBufferSize];
		// And save the width and height.
		texture->size = Vector2i(width, height);
		// Go through the whole image.
		for(int j = 0; j < height; j++){
		//	int y = height - j - 1;
			int y = j;
			for(int x = 0; x < width; x++){

				//get RGBA components
				unsigned char r = image[SOURCE_INDEX + 0]; //red
				unsigned char g = image[SOURCE_INDEX + 1]; //green
				unsigned char b = image[SOURCE_INDEX + 2]; //blue
				unsigned char a = image[SOURCE_INDEX + 3]; //alpha
				// Set them into the texture data array.
				texture->data[TARGET_INDEX + 0] = r;
				texture->data[TARGET_INDEX + 1] = g;
				texture->data[TARGET_INDEX + 2] = b;
				texture->data[TARGET_INDEX + 3] = a;
			}
		}
	}
	else if (bytesPerPixel == 3){
		// Allocate texture data array.
		texture->data = new unsigned char [width*height*4];
		// And save the width and height.
		texture->size = Vector2i(width, height);
		// Go through the whole image.
		for(int j = 0; j < height; j++){
		//	int y = height - j - 1;
			int y = j;
			for(int x = 0; x < width; x++){
				//get RGBA components
				unsigned char r = image[SOURCE_INDEX + 0]; //red
				unsigned char g = image[SOURCE_INDEX + 1]; //green
				unsigned char b = image[SOURCE_INDEX + 2]; //blue
				unsigned char a = 255; //alpha
				// Set them into the texture data array.
				int targetIndex = TARGET_INDEX;
				if (targetIndex < 0)
					std::cout<<"\n ;_;";
				else if (targetIndex > width * height * 4)
					std::cout<<"\n ;_;";
				texture->data[TARGET_INDEX + 0] = r;
				texture->data[TARGET_INDEX + 1] = g;
				texture->data[TARGET_INDEX + 2] = b;
				texture->data[TARGET_INDEX + 3] = a;
			}
		}
	}
	else {
#ifdef WINDOWS
        MessageBox(NULL, L"Needs to be 32.", L"Bytes per pixel invalid", MB_OK | MB_ICONEXCLAMATION);
		PostQuitMessage(3);
#endif
		return NULL;
	}
	/// Since we converted the image to RGBA above, fix the BitsPerPixel again!
	return true;
}
