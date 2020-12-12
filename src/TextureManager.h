/// Emil Hedemalm
/// 2014-08-10 (although older)
/// Texture manageeer

#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <Util.h>
#include "Texture.h"
#include "Color.h"

class Entity;
#define EntitySharedPtr std::shared_ptr<Entity>

#define TexMan		(*TextureManager::Instance())

class TextureManager{
private:
	/// Default constructor that nullifies all initial texture IDs
	TextureManager();
	static TextureManager * texMan;
public:
	static void Allocate();
	static TextureManager * Instance() { return texMan; };
	static void Deallocate();
	/// Destructor
	~TextureManager();

	/// Creates a new texture that the texture manager will make sure to deallocate once the program shuts down, so you don't have to worry about it.
	Texture * New();
	/// Creates a new texture, made for updating more than once.
	Texture * NewDynamic();
	/// Deletes target texture and its associated memory. The object should not be touched any more after calling this.
	void DeleteTexture(Texture * texture);
	/// Prints a list of all textures to console, starting with their ID
	void ListTextures();

	/// Reloads all textures.
	void ReloadTextures();
	/// Rebufferizes all textures. Call only from Render-thread.
	void RebufferizeTextures();


	/** Loads all textures from the provided source/name list.
		Returns amount of failed loadings.
	*/
	int LoadTextures(List<String> & texturesToLoad);
	/** Loads target texture into memory, queueing it's bufferization to the graphicsManager. Returns the Texture's address.
		If already loaded, returns a pointer to the pre-existing texture.
		By default textures are assumed to be located in the /img/ directory. If some other path is requested noPathAdditions should be set to true.
	*/
	Texture * LoadTexture(String source, bool noPathAdditions = false);
	/// Loads all required textures for the specified state into memory.   WHAT.. I don't even
	bool LoadTextures(int state);
	/// Loads all textures required by target Entity.
	bool LoadTextures(EntitySharedPtr Entity);

	/// Generates a texture with automatic name and given color. The texture will be exactly 1 or 2x2 pixels, simply for the color!
	Texture * GenerateTexture(const Color & andColor);
	/// Generates a texture with given name and color. The texture will be exactly 1 or 2x2 pixels, simply for the color!
	Texture * GenerateTexture(String withName, const Color & andColor);

	/// Buffers all textures required by a certain StateMan.
	bool BufferizeTextures(int state);
	/// Buffers all textures required by target Entity.
	bool BufferizeTextures(List<Texture*> textures);
	/// Buffers all textures required by target Entity.
	bool BufferizeTextures(EntitySharedPtr Entity);
	/** Bufferizes texture at target index in the texture array.
		Assertion will be thrown if outside the array.
	*/
	void BufferizeTexture(int index);
	/// Bufferizes target texture
	void BufferizeTexture(Texture * texture);
	/// Unbufferizes target texture
	void UnbufferizeTexture(Texture * texture);

	/// Checks if a texture exists by given name, returning it if so. Does NOT attempt to load any new textures.
	Texture * ExistsTextureByName(String name);

    /// Getter function that first tries to fetch texture by name, and if that failes tries to get it by it's source.
    Texture * GetTexture(String nameOrSource);
	/// Color o.o..
	Texture * GetTextureByColor(Color & color);
	/// 0xRRGGBB (red green blue)
	Texture * GetTextureByHex24(uint32 hexColor);
	/// 0xRRGGBBAA (red green blue alpha)
	Texture * GetTextureByHex32(uint32 hexColor);
	/// Gets texture with specified name. This assumes each texture has gotten a unique name.
	Texture * GetTextureByName(String name);
	/// Returns texture in the list by specified index.
	Texture * GetTextureByIndex(int index);
	/// Gets texture with specified source. Returns 0 if it could not be found.
	Texture * GetTextureBySource(String source);
	/// For buffering
	Texture * GetTextureByID(int glid);

	/// Frees the GL allocated IDs/memory of all textures.
	void FreeTextures();

	/// Checks if target image is supported for loading by the game engine.
	bool SupportedImageFileType(String fileName);

private:
	/// Attempts to load a texture using OpenCV imread.
	bool LoadTextureOpenCV(String source, Texture * texture);
	/// Attempts to load a texture using LodePNG library.
	bool LoadTextureLodePNG(String source, Texture * texture);

	/// Textures used by the manager
	List<Texture*> textures;
};

#endif
