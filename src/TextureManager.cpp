#include "TextureManager.h"

#include <fstream>
#include <iostream>

#include "Graphics/Messages/GraphicsMessage.h"
#include "Graphics/GraphicsManager.h"

#include "OS/Sleep.h"

/// Texture manager for the application.
TextureManager * TextureManager::texMan = NULL;

/// Allocate
void TextureManager::Allocate(){
	assert(texMan == NULL);
	texMan = new TextureManager();
}
void TextureManager::Deallocate(){
	assert(texMan);
	delete(texMan);
	texMan = NULL;
}

TextureManager::TextureManager(){}

TextureManager::~TextureManager(){
	textures.ClearAndDelete();
}

void TextureManager::DeallocateTextures(){
	for (int i = 0; i < textures.Size(); ++i){
		if (textures[i]->glid){
			glDeleteTextures(1, &textures[i]->glid);
			// Bad IDs must  be -1!!!
			textures[i]->glid = -1;
		}
	}
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
    return tex;
}

/// Gets texture with specified name. This assumes each texture has gotten a unique name.
Texture * TextureManager::GetTextureByName(String name){
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

	std::cout<<"\nTexture not loaded, attempting to load it.";
	return LoadTexture(name);
}

/// Returns texture in the list by specified index.
Texture * TextureManager::GetTextureByIndex(int index){
	if (index < 0 || index > textures.Size() -1)
		return NULL;
	return textures[index];
}

Texture * TextureManager::GetTextureBySource(String source){
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
	std::cout<<"\nTexture not loaded, attempting to load it.";
	return LoadTexture(source);
}

/// For buffering
Texture * TextureManager::GetTextureByID(int glid){
	for (int i = 0; i < textures.Size(); ++i)
		if (textures[i]->glid == glid)
			return textures[i];
	return NULL;
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

/// Generates a texture with given name and color. The texture will be exactly 1 or 2x2 pixels, simply for the color!
Texture * TextureManager::GenerateTexture(String withName, Vector4f andColor)
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


	textures.Add(newTex);
	return newTex;
}


#include <vector>
#include "LodePNG/lodepng.h"
#include "Globals.h"

Texture * TextureManager::LoadTexture(String source){
	source = FilePath::MakeRelative(source);
	for (int i = 0; i < textures.Size(); ++i){
		if (textures[i]->source.Contains(source) ||
			source.Contains(textures[i]->source)){
			std::cout<<"\nTexture \""<<source<<"\" already loaded, skipping.";
			return textures[i];
		}
	}
	if (!(source.Contains("img/") || source.Contains("img\\")
		|| source.Contains("anim/") || source.Contains("anim\\")
		|| source.Contains(":\\") || source.Contains(":/")))
		source = "img/" + source;

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
Sleep(100);
	//		std::cout<<"\nERROR: File "<<source.c_str()<<" does not exist!";
Sleep(100);
	//		assert("TextureManager::LoadTexture: ERROR: File <<source<< does not exist!");
Sleep(100);
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

	/// Create the texture now that we have decoded the data!
	Texture * texture = new Texture();

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

/*
	/// Flip the texture!
	std::cout<<"\nFlipping texture "<<source<<"!";
	for (unsigned int i = 0; i < height/2; ++i){
		for (unsigned int j = 0; j < width; ++j){
			for (int s = 0; s < 4; ++s){
				unsigned char tmp = data->data[i * width * 4 + j * 4 + s];
				data->data[i * width * 4 + j * 4 + s] = data->data[(height - i - 1) * width * 4 + j * 4 + s];
				data->data[(height - i - 1) * width * 4 + j * 4 + s] = tmp;
			}
		}
	}
*/


	/*
	glGenTextures(1, &texture[freeSlot].glid);
	glBindTexture(GL_TEXTURE_2D, texture[freeSlot].glid);  // Bind glTexture ID.


	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, data.data);
	{
		GLuint error = glGetError();
		if (error != GL_NO_ERROR){
			print("\nGL Error in TextureManager::LoadTexture: ");
			print(error);
		}
	}
	*/

	// Save other texture details now before returning.
	texture->SetSource(source);
	texture->SetName(source);
	std::cout<<"\nTexture loaded as: "<<texture->name;

	textures.Add(texture);
	/// Queue the texture for bufferization. This may want to be adjusted somewhere else maybe.
	/// If queueing from within the manager we will thread-deadlock. Better buffer it right before rendering..!
///	Graphics.QueueMessage(new GMBufferTexture(texture));
	return texture;
}

void TextureManager::BufferizeTexture(int index){
	if (index >= 0 && index < textures.Size()){
		BufferizeTexture(textures[index]);
	}
	assert(false && "index outside valid range in TextureManager::BufferizeTexture");
}

void TextureManager::BufferizeTexture(Texture * texture){
	if (texture == NULL){
		assert(texture && "NULL texture provided in TextureManager::BufferizeTexture, skipping it!");
		return;
	}
	if (texture->glid != -1){
//		std::cout<<"\nTexture \""<<texture->source<<"\" already bufferized! Skipping.";
		return;
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

	std::cout<<"\nBuffering texture "<<texture->name<<"...";
	// Generate texture
	glGenTextures(1, &texture->glid);

	// Set texturing parameters
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	// Enable texturing
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture->glid);  // Bind glTexture ID.

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Generate the texture Entity in GL
	// Ref: http://www.opengl.org/sdk/docs/man/xhtml/glTexImage2D.xml
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,	texture->width,
		texture->height,		0, 	GL_RGBA, GL_UNSIGNED_BYTE, texture->data);

	/// Generate mip-maps!
	glGenerateMipmap(GL_TEXTURE_2D);

	/*
	int initialWidth = texture->width, initialHeight = texture->height;
	int tmpWidth = initialWidth, tmpHeight = initialHeight;
	std::cout<<"\n0. Width: "<<tmpWidth<<" height: "<<tmpHeight;
	for (int i = 1; i < 14; ++i){
		tmpWidth /= 2;
		tmpHeight /= 2;
		std::cout<<"\n"<<i<<". Width: "<<tmpWidth<<" height: "<<tmpHeight;
		if (tmpWidth <= 1 && tmpHeight <= 1)
			break;
		glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA,	tmpWidth, tmpHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->data);
		int error = glGetError();
		for (int i = 0; i < texture->dataLength; ++i)
			texture->data[i] /= 2;
		if (error != GL_NO_ERROR){
			std::cout<<"\nError: "<<error;
		}
	}
	*/

	error = glGetError();
	if (error != GL_NO_ERROR){
		std::cout<<"\nGL Error in TextureManager::BufferizeTexture: "<<error;
	}


	/// Deallocate data from memory if it is no longer needed?
/*	int lastRed = 0;
	int newRed = 0;
	print("\nReds: ");
	for (int i = 0; i < texture->data.height; ++i){
		for (int j = 0; j < texture->data.width; ++j){
			newRed = texture->data.data[(i*texture->data.width + j) * 4];
			if (newRed != lastRed){
				print(newRed<<" ");
				lastRed = newRed;
			}
		}
	}
	*/
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
