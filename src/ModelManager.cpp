#include "ModelManager.h"

#include <iostream>
#include <cstdlib>
//#include <windows.h>

#include "File/FileUtil.h"
#include "String/StringUtil.h"

#include "ObjReader.h"
#include "Model.h"
#include "XML/XMLParser.h"
#include "Model/ColladaImporter.h"

// Static model manager singleton
ModelManager * ModelManager::modelManager = NULL;

/// Allocate
void ModelManager::Allocate(){
	assert(modelManager == NULL);
	modelManager = new ModelManager();
}
ModelManager * ModelManager::Instance(){
	assert(modelManager);
	return modelManager;
}
void ModelManager::Deallocate(){
	assert(modelManager);
	delete(modelManager);
	modelManager = NULL;
}

ModelManager::ModelManager(){
	idEnumerator = 1;
	defaultTexture = NULL;
}

ModelManager::~ModelManager()
{
	modelList.ClearAndDelete();
}

/// Loads required models (either hard-coded or from file)
void ModelManager::Initialize(){
	LoadObj("sphere.obj");
	LoadObj("cube.obj");
	LoadObj("plane.obj");
	LoadObj("Sphere6.obj");
	LoadObj("Awesome haus_uv.obj");
};

/// Returns pointer to model at target index
Model * ModelManager::GetModel(int index){
	if (index < 0 || index >= modelList.Size())
		return NULL;
	return modelList[index];
}

/// Returns a pointer to model with target name, loads it if it wasn't found.
Model * ModelManager::GetModel(String name){
	name = FilePath::MakeRelative(name);
//    std::cout<<"\nGetModel: "<<name;
	for (int i = 0; i < modelList.Size(); ++i){
		String modelName = modelList[i]->Name();
 //       std::cout<<"\nModel name: "<<modelName;
		if (modelList[i]->Name().Contains(name))
			return modelList[i];
	}
	std::cout<<"\nINFO: Unable to load model "<<name<<", trying to load from file.";
	Model * newModel = LoadObj(name);
	return newModel;
}

/// Prints a list of all objects to console, starting with their ID
void ModelManager::ListObjects(){
	std::cout<<"\nListing models: ";
	for (int i = 0; i < modelList.Size(); ++i){
		if (modelList[i]->name){
			std::cout<<"\n"<<i<<". "<<modelList[i]->name;
		}
	}
}

/** Attempts to load all models using the provided source list.
	Returns amount of failed loadings.
*/
int ModelManager::LoadModels(List<String> modelSourceList){
	int failed = modelSourceList.Size();
	std::cout<<"\nLoadModels called for "<<modelSourceList.Size()<<" models.";
	for (int i = 0; i < modelSourceList.Size(); ++i){
        std::cout<<"\n- loading "<<modelSourceList[i]<<"...";
		if (!LoadObj(modelSourceList[i].c_str()))
			--failed;
	}
	return failed;
}

/// Loads Model with single Mesh from file. Used primarily for static objects.
Model * ModelManager::LoadObj(String source)
{
	assert(source && "Null source sent into ModelManager::LoadObj!");
	source = FilePath::MakeRelative(source);
	// Check if it pre-exists
	for (int i = 0; i < modelList.Size(); ++i){
		Model * model = modelList[i];
		std::cout<<"\nModel source: "<<model->source;
		if (model->mesh->source.Contains(source) || source.Contains(model->mesh->source)){
			std::cout<<"\nObject already loaded, returning a pointer to it!";
			return modelList[i];
		}
	}
	/// Add obj/ before and .obj at end if needed
	if (!(source.Contains("obj/") ||
		source.Contains("obj\\") ||
		source.Contains(":")))
	{
		source = "obj/" + source;
	}
	if (!source.Contains("."))
	{
		source += ".obj";
	}

	Mesh * mesh = NULL;
	std::cout<<"\nCreating mesh.";
	mesh = new Mesh();
	

	// Check if a compressed version exists.
	String compressedPath = source;
	compressedPath.Remove("obj/");
	compressedPath = "CompressedObj/" + compressedPath;
	// Ensure folders exist for the path.
	List<String> pathFolders = compressedPath.Tokenize("/");
	pathFolders.RemoveIndex(pathFolders.Size() - 1);
	String compressedFolderPath = MergeLines(pathFolders, "/");
	CreateDirectoriesForPath(compressedFolderPath);
	compressedPath.Remove(".obj");
	compressedPath += ".cobj";
	bool compressedLoadResult = mesh->LoadCompressedFrom(compressedPath);
	bool modelLoaded = false;
	if (compressedLoadResult)
	{
		// ... 
		modelLoaded = true;
	}
	// Load regular model.
	else {

	   /// Try opening the file first.
		std::fstream file;
		file.open(source.c_str());
		if (!file.is_open()){
			std::cout<<"\nUnable to open file stream to "<<source;
			file.close();
			// Delete mesh if not needed.
			delete mesh;
			return NULL;
		}
		file.close();

	  //  path.Replace('\\', '/'); // Replace bad folder slashes with good-'uns!
		std::cout<<"\nLoading model from source: "<<source;
		std::cout<<"\nCalling ObjReader::ReadObj";
		modelLoaded = ObjReader::ReadObj(source.c_str(), mesh);
	 //   mesh->PrintContents();

	}
	// If model was loaded, create new model for it.
	if (modelLoaded){
		Model * model = new Model();
		model->mesh = mesh;
		
		// Calculate bounds, and set bounds since this function assumes static/single mesh-models!
		if (mesh->max.MaxPart() == 0 && mesh->min.MaxPart() == 0)
			mesh->CalculateBounds();

		model->aabb = AxisAlignedBoundingBox(mesh->min, mesh->max);

		// Centering can be bad to configuring stuff yourself..!
	//	mesh->Center();
		model->radius = mesh->radius;
		model->centerOfModel = mesh->centerOfMesh;
		model->SetName(source);
		model->source = source;
		std::cout<<" .obj successfully read!";

		/// Create the triangulated one straight away~, if needed..!
		if (!model->mesh->IsTriangulated())
		{
			std::cout<<"\nCreating triangulized mesh..";
			model->triangulizedMesh = new Mesh();
			model->triangulizedMesh->LoadDataFrom(model->mesh);
			std::cout<<"\nTriangulized mesh finished o-o.";
			std::cout<<"\nTriangulate.";
			model->triangulizedMesh->Triangulate();
			std::cout<<"\nTriangulated.";
			if (model->triangulizedMesh->normal == NULL)
				model->triangulizedMesh->RecalculateNormals();
			std::cout<<"\nNormalized";
//			model->triangulizedMesh->CalculateUVTangents();
	//		std::cout<<"\nUVd";

			// Save the triangulized mesh in compressed form !
			model->triangulizedMesh->SaveCompressedTo(compressedPath);
		}

		modelList.Add(model);
		return model;
	}
	else {
	    std::cout<<"\nUnable to load mesh, trying to delete allocated content.";
		delete mesh;
		mesh = NULL;
		return NULL;
	}
	std::cout<<"\nERROR: Did not manage to read/load file.";

	return NULL;
}


/// Loads a model using target Collada file, using all given geometry nodes within it to generate a single mesh.
Model * ModelManager::LoadCollada(String source){
	assert(source && "Null source sent into ModelManager::LoadCollada!");
	source = FilePath::MakeRelative(source);
	// Check if it pre-exists
	for (int i = 0; i < modelList.Size(); ++i){
		Model * model = modelList[i];
		if (model->mesh->source.Contains(source) || source.Contains(model->mesh->source)){
			std::cout<<"\nCollada .DAE already loaded, returning a pointer to it!";
			return modelList[i];
		}
	}
	/// Add obj/ before and .obj at end if needed
	if (!(source.Contains("dae/") ||
		source.Contains("dae\\") ||
		source.Contains(":")))
	{
		source = "dae/" + source;
	}
	if (!source.Contains("."))
	{
		source += ".dae";
	}

	/// Try opening the file first.
	std::fstream file;
	file.open(source.c_str());
	if (!file.is_open()){
		std::cout<<"\nUnable to open file stream to "<<source;
		file.close();
		return NULL;
	}
	file.close();

	//  path.Replace('\\', '/'); // Replace bad folder slashes with good-'uns!
	std::cout<<"\nLoading model from source: "<<source;
	Mesh * mesh = NULL;
	ColladaImporter colladaImporter;
	colladaImporter.Load(source);

	List<String> geometries = colladaImporter.Geometries();
	assert(geometries.Size() == 1);
	mesh = colladaImporter.CreateMesh(geometries[0]);

	assert(mesh);
	if (mesh == NULL){
		std::cout<<"\nERROR: Unable to load file in ModelManager::LoadCollada()";
		return NULL;
	}

	Model * model = new Model();
	model->mesh = mesh;
	// Calculate bounds, and set bounds since this function assumes static/single mesh-models!
	mesh->CalculateBounds();
	// Centering can be bad to configuring stuff yourself..!
	//	mesh->Center();
	model->radius = mesh->radius;
	model->centerOfModel = mesh->centerOfMesh;
	model->SetName(source);
	std::cout<<" .dae successfully read!";

	/// Create the triangulated one straight away~
	std::cout<<"\nCreating triangulized mesh..";
	model->triangulizedMesh = new Mesh();
	model->triangulizedMesh->LoadDataFrom(model->mesh);
	std::cout<<"\nTriangulized mesh finished o-o.";
	std::cout<<"\nTriangulate.";
	model->triangulizedMesh->Triangulate();
	std::cout<<"\nTriangulated.";
	if (model->triangulizedMesh->normal == NULL)
		model->triangulizedMesh->RecalculateNormals();
	std::cout<<"\nNormalized";
	model->triangulizedMesh->CalculateUVTangents();
	std::cout<<"\nUVd";
	modelList.Add(model);
	return model;
}


// Removes target Entity
/*
bool ModelManager::removeObject(Model * i_object){
	for (int i = 0; i < MAX_MODELS; ++i){
		if (model[i] == i_object){
			delete model[i];
			model[i] = NULL;
			return true;
		}
	}
	return false;
}
*/
