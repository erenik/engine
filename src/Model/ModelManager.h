/// Emil Hedemalm
/// 2014-07-27
/// A manager for all models encapsulating 3D-data.

#ifndef MODEL_MANAGER_H
#define MODEL_MANAGER_H

#include "Util.h"

class Model;
class Texture;

/// Maximum amount of simultaneously loaded objects
const int MAX_MODELS = 1000;

#define ModelMan	(*ModelManager::Instance())

/** A handler class for all object types.
	Do note that an object can have several different meshes (for animation, physics, etc.)
	A given model is valid as long as it has a name.
*/
class ModelManager {
	/// Default constructor
	ModelManager();
	static ModelManager * modelManager;
public:
	static void Allocate();
	static ModelManager * Instance();
	static void Deallocate();
	~ModelManager();
	/// Loads required models (either hard-coded or from file)
	void Initialize();


	/// Creates a new model that may be dynamically manipulated and re-buffered as editing proceeds.
	Model * NewDynamic();

	/// Returns pointer to model at target index
	Model * GetModel(int index);
	/// Returns a pointer to model with target name
	Model * GetModel(String name);
	/// Prints a list of all objects to console, starting with their ID
	void ListObjects();
	/** Attempts to load all models using the provided source list.
		Returns amount of failed loadings.
	*/
	int LoadModels(List<String> modelSourceList);
	/** Loads Model with single Mesh from file.
		Used primarily for static objects. */
	Model * LoadObj(String source);
	
	/// Loads a model using target Collada file, using all given geometry nodes within it to generate a single mesh.
	Model * LoadCollada(String source);

	/** Removes target Entity    */
//	bool removeObject(EntitySharedPtr Entity);


	/// Default texture for newly constructed objects.
	Texture * defaultTexture;

private:
	/// Array for all objects
	List<Model*> modelList;
	/// Id counter for generating unique id's
	int idEnumerator;
};


#endif
