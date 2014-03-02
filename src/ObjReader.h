/**
Abused file: http://3dexport.com/3dmodel-wooden-box-open-39443.htm
*/

#ifndef OBJREADER_H
#define OBJREADER_H

#include <GL/glew.h>
#include "Mesh.h"

#include <fstream>
#include <sstream>

/// Custom OBJ-reader class.
class ObjReader {
public:
	/** Attempts to read an OBJ-file from the specified filename. If successful the mesh is loaded into the provided mesh.
		Vertex Buffer Objects (VBOs) are generated automatically upon loading, and their respective IDs are stored in the mesh as well.
	*/
	static bool ReadObj(const char * filename, Mesh * mesh);

};

#endif
