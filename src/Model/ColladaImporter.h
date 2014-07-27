// Emil Hedemalm
// 2013-07-21

#ifndef COLLADA_IMPORTER_H
#define COLLADA_IMPORTER_H

#include "List/List.h"
#include "String/AEString.h"

class XMLParser;
class XMLElement;
class Mesh;

enum axises{
	Y_UP,
	Z_UP,
};

// Mainly aimed to import animation if possibru.
class ColladaImporter {
public:
	ColladaImporter();
	~ColladaImporter();

	/// Attempts to load from file. All content will be stored internally until a build-command is issued.
	bool Load(String fromFile);

	/// Returns a list of all data that was parsed.
	List<String> Data();
	List<String> Meshes();
	List<String> Geometries();

	/// Creates the named mesh (a reference must exist within the parsed data).
	Mesh * CreateMesh(String name);
private:

	XMLElement * GetGeometry(String byName);

	/// Pursur.
	XMLParser * parser;


	/// List with all geometries.
	List<XMLElement*> geometryList;

	// Basic info.
	String authoring_tool;
	float unit; // 1.0 for meter, 0.01 for cm, etc.
	int up_axis;


};

#endif