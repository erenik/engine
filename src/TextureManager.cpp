/// Emil Hedemalm
/// 2014-08-10 (although older)
/// Texture manageeer

#include "Libs.h"

#include "TextureManager.h"

#include <fstream>
#include <iostream>

#include "File/LogFile.h"
#include "File/FileUtil.h"
#include "Graphics/Messages/GraphicsMessage.h"
#include "Graphics/GraphicsManager.h"

#include "OS/Sleep.h"

#include <vector>
#include "LodePNG/lodepng.h"
#include "Globals.h"
#include "Color.h"

#ifdef OPENCV
#include "opencv2/opencv.hpp"
#endif

#include "Time/Time.h"

/// Texture manager for the application.
TextureManager * TextureManager::texMan = NULL;

/// Allocate
void TextureManager::Allocate(){
	assert(texMan == NULL);
	texMan = new TextureManager();
}
void TextureManager::Deallocate()
{
	assert(texMan);
	delete(texMan);
	texMan = NULL;
}

TextureManager::TextureManager(){}

TextureManager::~TextureManager()
{
	textures.ClearAndDelete();
}

/// Getter function that first tries to fetch texture by name, and if that failes tries to get it by it's source.
Texture * TextureManager::GetTexture(String nameOrSource)
{
    Texture * tex = NULL;
	if (nameOrSource.Type() == String::WIDE_CHAR)
		nameOrSource.ConvertToChar();
	// Delete whitespace..
	nameOrSource.RemoveInitialWhitespaces();
    tex = GetTextureByName(nameOrSource);
    if (!tex)
        tex = GetTextureBySource(nameOrSource);

	if (tex)
	{
		if (tex->source.Length() == 0)
			tex->SetSource(nameOrSource);
	}
	if (tex)
		assert(tex->name.Length());
    return tex;
}

/// Color o.o..
Texture * TextureManager::GetTextureByColor(Color & color)
{
	return GenerateTexture(color.GetName(), color);	
}


/// 0xRRGGBBAA (red green blue alpha)
Texture * TextureManager::GetTextureByHex24(uint32 hexColor)
{
	int alphaBit = 255;
	int alpha = 0xFF;
	uint32 hexColor32 = hexColor << 8;
	hexColor32 += alpha;
	unsigned char r, g, b, a;
	r = hexColor32 >> 24 % 256;
	g = hexColor32 >> 16 % 256;
	b = hexColor32 >> 8 % 256;
	a = hexColor32 >> 0 % 256;	
	return GetTextureByHex32(hexColor32);
}

/// 0xRRGGBBAA (red green blue alpha)
Texture * TextureManager::GetTextureByHex32(uint32 hexColor)
{
	String texName = String::ToHexString(hexColor);
	Texture * tex = NULL;
	tex = GetTextureByName(texName);
	if (!tex)
	{
		tex = New();
		tex->name = texName;
		tex->SetSize(Vector2i(1,1));
		unsigned char r, g, b, a;
		r = hexColor >> 24 % 256;
		g = hexColor >> 16 % 256;
		b = hexColor >> 8 % 256;
		a = hexColor >> 0 % 256;
		Vector4f color(r,g,b,a);
		tex->SetColor(color);
	}
	return tex;
}

/// Gets texture with specified name. This assumes each texture has gotten a unique name.
Texture * TextureManager::GetTextureByName(String name)
{
	if (name.Length() == 0)
		return NULL;
	name = FilePath::MakeRelative(name);
	int queriedLength = name.Length();
	for (int i = 0; i < textures.Size(); ++i)
	{
		String texName = textures[i]->name;
		if (texName == name)
			return textures[i];
	}
	/// Check if the name has a file-ending. If not, assume it's a general color!
	if (name == "Black")
		return GenerateTexture("Black", Vector4f(0,0,0,1));
	else if (name == "Grey" || name == "Gray")
        return GenerateTexture("Grey", Vector4f(0.5f,0.5f,0.5f,1.0f));
	else if (name == "White")
        return GenerateTexture("White", Vector4f(1.f,1.f,1.f,1.f));
	else if (name == "NULL" || name == "Alpha")
		return GenerateTexture("NULL", Vector4f(1,1,1,0));
	else if (name == "Red")
		return GenerateTexture("Red", Vector4f(1,0,0,1));
	else if (name == "Green")
		return GenerateTexture("Green", Vector4f(0,1,0,1));
	else if (name == "Blue")
		return GenerateTexture("Blue", Vector4f(0,0,1,1));
	else if (name == "Cyan")
		return GenerateTexture("Cyan", Vector4f(0,1,1,1));
	/// Hex-code go!
	else if (name.Contains("0x"))
	{
		return GenerateTexture(name, Color::ColorByHexName(name));
	}
	

//	std::cout<<"\nTexture not loaded, attempting to load it.";
	return NULL; // LoadTexture(name);
}

/// Returns texture in the list by specified index.
Texture * TextureManager::GetTextureByIndex(int index){
	if (index < 0 || index > textures.Size() -1)
		return NULL;
	return textures[index];
}

Texture * TextureManager::GetTextureBySource(String source)
{
	if (source == 0)
		return NULL;
	source = FilePath::MakeRelative(source);
	for (int i = 0; i < textures.Size(); ++i){
		Texture * tex = textures[i];
		if (tex->source.Contains(source) ||
			source.Contains(tex->source)){
				return tex;
		}
	}
//	std::cout<<"\nTexture not loaded, attempting to load it.";
	return LoadTexture(source);
}

/// For buffering
Texture * TextureManager::GetTextureByID(int glid){
	for (int i = 0; i < textures.Size(); ++i)
		if (textures[i]->glid == glid)
			return textures[i];
	return NULL;
}

/// Frees the GL allocated IDs/memory of all textures.
void TextureManager::FreeTextures()
{
	std::cout<<"\nFreeing "<<textures.Size()<<" textures";
	int freed = 0;
	for (int i = 0; i < textures.Size(); ++i)
	{
		Texture * texture = textures[i];
		if (texture->glid == -1)
			continue;
		glDeleteTextures(1, &texture->glid);
		texture->glid = -1;
		++freed;
	}
	std::cout<<freed<<" textures freed. ";
}


/// Creates a new texture that the texture manager will make sure to deallocate once the program shuts down, so you don't have to worry about it.
Texture * TextureManager::New()
{
	Texture * tex = new Texture();
	textures.Add(tex);
	tex->format = Texture::RGBA;
	tex->bpp = 4;
	return tex;
}

/// Creates a new texture, made for updating more than once.
Texture * TextureManager::NewDynamic()
{
	Texture * tex = New();
	tex->dynamic = true;
	return tex;
}

/// Deletes target texture and its associated memory. The object should not be touched any more after calling this.
void TextureManager::DeleteTexture(Texture * texture)
{
	textures.Remove(texture);
	delete texture;
}


/// Prints a list of all objects to console, starting with their ID
void TextureManager::ListTextures(){
	std::cout<<"\nListing textures: ";
	for (int i = 0; i < textures.Size(); ++i){
		std::cout<<"\n"<<i<<". "<<textures[i]->name;
	}
}

/// Loads all textures from the provided source/name list.
int TextureManager::LoadTextures(List<String> & texturesToLoad){
	int failed = texturesToLoad.Size();
	for (int i = 0; i < texturesToLoad.Size(); ++i){
		if (!LoadTexture(texturesToLoad[i].c_str()))
			++failed;
	}
	return failed;
}

/// Generates a texture with automatic name and given color. The texture will be exactly 1 or 2x2 pixels, simply for the color!
Texture * TextureManager::GenerateTexture(const Vector4f & andColor)
{
	String name = "GeneratedTexture: r"+String(andColor[0])+" g"+String(andColor[1])+" b"+String(andColor[2]);
	return GenerateTexture(name, andColor);
}

/// Generates a texture with given name and color. The texture will be exactly 1 or 2x2 pixels, simply for the color!
Texture * TextureManager::GenerateTexture(String withName, const Vector4f & andColor)
{
	/// Check that we don't already have one with the same name, it should be correct if so.
	for (int i = 0; i < textures.Size(); ++i){
		Texture * tex = textures[i];
		if (tex->name == withName)
			return tex;
	}
	Texture * newTex = new Texture();
	newTex->width = newTex->height = 1;
	newTex->bpp = 4;
	newTex->format = Texture::RGBA;
	newTex->CreateDataBuffer();
	newTex->SetColor(andColor);
	newTex->name = withName;
	newTex->source = "Generated";


	textures.Add(newTex);
	return newTex;
}

/// Buffers all textures required by target Entity.
bool TextureManager::BufferizeTextures(List<Texture*> textures)
{
	int ok = 0;
	for (int i = 0; i < textures.Size(); ++i)
	{
		Texture * tex = textures[i];
		if (!tex)
			continue;
		ok += tex->Bufferize();
	}
	return true;
}

Texture * TextureManager::LoadTexture(String source, bool noPathAdditions)
{
	source = FilePath::MakeRelative(source);
	for (int i = 0; i < textures.Size(); ++i){
		if (textures[i]->source.Contains(source) ||
			source.Contains(textures[i]->source)){
//			std::cout<<"\nTexture \""<<source<<"\" already loaded, skipping.";
			return textures[i];
		}
	}
	if (!noPathAdditions)
	{
		if (!(source.Contains("img/") || source.Contains("img\\")
			|| source.Contains("anim/") || source.Contains("anim\\")
			|| source.Contains(":\\") || source.Contains(":/")))
			source = "img/" + source;
	}
	if (!source.Contains("."))
		source = source + ".png";
    source.Replace('\\', '/');
    if (source.Contains("/bin")){
        std::cout<<"\nSource contains \"GameEngine\" string. Remove it and all before it~";
        std::cout<<"\nFrom: "<<source<<" ";
        List<String> tokens = source.Tokenize("/");
        std::cout<<"\nTokens: "<<tokens.Size();
        for (int i = 0; i < tokens.Size(); ++i){
            std::cout<<"\nToken "<<i<<": "<<tokens[i];
            if (tokens[i] == "bin"){
                // Rebuild
                std::cout<<"\nBeginning rebuild..";
                source = "";
                for (int j = i+1; j < tokens.Size(); ++j){
                    std::cout<<"\nAdding "<<tokens[j];
                    source += tokens[j];
                    std::cout<<"\nSource: "<<source;
                    if (j < tokens.Size()-1){
                        source += "/";
                        std::cout<<"\nAdding folder /";
                    }
                }
                break; // n break loop
            }
        }
        std::cout<<": "<<source;
    }


	std::cout<<"\nLoading texture \""<<source.c_str()<<"\"...";


	if (!FileExists(source))
	{
		LogMain("TextureManager::LoadTexture: No such file "+source, INFO);
		return NULL;
	}

	Texture * texture = new Texture();
	// resize texture so the comparison gets valid...
	Timer timer;
	timer.Start();

	// Prefer OpenCV since it's faster.
	bool ok;
	ok = LoadTextureOpenCV(source, texture);
	if (!ok)
	{
		std::cout<<"\nOpenCV failed to read texture D:";
		delete texture;
		return NULL;
	}

	/*
	bool ok = LoadTextureLodePNG(source, texture);
	if (!ok)
	{
		delete texture;
		return NULL;
	}
	int64 first = timer.GetMicro();	

	// resize texture so the comparison gets valid...
	delete[] texture->data;
	texture->data = 0;
	texture->width = texture->height = 0;

	timer.Start();
	int64 second = timer.GetMicro();

	std::cout<<"\nLodePNG: "<<first * 0.000001f<<" OpenCV: "<<second * 0.000001f;
*/

	// Save other texture details now before returning.
	texture->SetSource(source);
	texture->SetName(source);
	std::cout<<" done.";

	textures.Add(texture);
	/// Queue the texture for bufferization. This may want to be adjusted somewhere else maybe.
	/// If queueing from within the manager we will thread-deadlock. Better buffer it right before rendering..!
///	Graphics.QueueMessage(new GMBufferTexture(texture));
	return texture;
}



/// Checks if target image is supported for loading by the game engine.
bool TextureManager::SupportedImageFileType(String fileName)
{
	/// Supported via OpenCV: http://docs.opencv.org/modules/highgui/doc/reading_and_writing_images_and_video.html?highlight=imread#imread
//	List<String> extensions;
	List<String> extensions;
	extensions.Add(".png", ".bmp", ".jpg");
	extensions.Add(".jpeg", ".tif", ".tiff");
	/*
	char * extensions [7] = {
		".png", ".bmp", ".dib", 
		".tif", ".tiff", ".jpg", 
		".jpeg"
	};
	*/
//	extensions.Add(7, cExtentions);
	for (int i = 0; i < 7; ++i)
	{
		if (fileName.Contains(extensions[i]))
			return true;
	}
	return false;
}		

/// Attempts to load a texture using OpenCV imread.
bool TextureManager::LoadTextureOpenCV(String source, Texture * texture)
{
#ifdef OPENCV

	/// Supported via OpenCV: http://docs.opencv.org/modules/highgui/doc/reading_and_writing_images_and_video.html?highlight=imread#imread
	cv::Mat mat;
	mat = cv::imread(source.c_str(), CV_LOAD_IMAGE_UNCHANGED);
	if (!mat.cols || !mat.rows)
		return false;

	texture->source = source;
	texture->name = source;

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
#endif
	return true;
}


bool TextureManager::LoadTextureLodePNG(String source, Texture * texture)
{
	/// Obsoleted?
	/*

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
	texture->format = Texture::RGBA;

	// Get width, height and pixels
	int width = decoder.getWidth(), height =  decoder.getHeight();
	int bytesPerPixel = decoder.getBpp()/8;
	texture->bpp = bytesPerPixel;

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
		texture->width = width;
		texture->height = height;
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
		texture->width = width;
		texture->height = height;
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
		texture->width = width;
		texture->height = height;
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
	texture->bpp = 4;
	*/
	return true;
}

void TextureManager::BufferizeTexture(int index){
	if (index >= 0 && index < textures.Size()){
		BufferizeTexture(textures[index]);
	}
	assert(false && "index outside valid range in TextureManager::BufferizeTexture");
}

void TextureManager::BufferizeTexture(Texture * texture)
{
	texture->Bufferize();
}


/// Unbufferizes target texture
void TextureManager::UnbufferizeTexture(Texture * texture){
	if (texture->glid != -1)
		glDeleteTextures(1, &texture->glid);
	// Bad IDs must  be -1!!!
	texture->glid = -1;
}


/// Checks if a texture exists by given name, returning it if so. Does NOT attempt to load any new textures.
Texture * TextureManager::ExistsTextureByName(String name)
{
	for (int i = 0; i < textures.Size(); ++i)
	{
		Texture * tex = textures[i];
		if (tex->name == name)
			return tex;
	}
	return NULL;
}
