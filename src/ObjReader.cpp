// Emil Hedemalm
// 2013-07-03
// Swapped out std:: class usage for own one in order to handle new .obj files that were failing to parse.

#include "ObjReader.h"
#include <iostream>
#include <cstring>
#include "Timer/Timer.h"

// #include <GL/glew.h>
#include "Mesh/Mesh.h"


bool ObjReader::ReadObj(const char * filename, Mesh * mesh)
{
	std::cout<<"\n\nReadObj called for file: "<<filename;
	Timer t;
	t.Start();
	/// Make sure the mesh doesn't already contain data?
	char * data;
	// Size of read data
	int size;

	int numVertices = 0;
	int uvCoords = 0;
	int numNormals = 0;
	int numFaces = 0;

	try {

		std::fstream fileStream;
		fileStream.open(filename, std::ios_base::in);
		if (!fileStream.is_open()){
		    std::cout<<"\nUnable to open stream to file "<<filename<<" in ObjReader";
			return false;
        }

		int start  = (int) fileStream.tellg();

		// Get size by seeking to end of file
		fileStream.seekg( 0, std::ios::end );
		size = (int) fileStream.tellg();

		// Allocate data array for length
		data = new char [size+5];
		memset(data, 0, size+5);

		// Go to beginning of file and read the data
		fileStream.seekg( 0, std::ios::beg);
		fileStream.read((char*) data, size);

		// Then close the stream
		fileStream.close();
	} catch (...){
		return false;
	}
	int64 allocationTime = t.GetMs();
	std::cout<<"\nFile load raw data buffer allocationTime: "<<allocationTime;


	t.Start();
	/// Try with our own string-class too.
	String stringData = data;
	List<String> lines = stringData.GetLines();
	int64 time = t.GetMs();
	std::cout<<"\nAllocating stringData and List<String> lines out of the raw data: "<<time;
	t.Start();
	/// Now delete the data, too.
	delete[] data;
	data = NULL;
	int64 deallocationTime = t.GetMs();
	std::cout<<"\nDeallocation time for raw char[] data: "<<deallocationTime;

	t.Start();
	/// Perform initial count-parse.
	bool newSearch = true;
	bool oldSearch = false;
	if (newSearch){
		for (int i = 0; i < lines.Size(); ++i)
		{
			String line = lines[i];
			if (line.StartsWith("v "))
				++numVertices;
			else if (line.StartsWith("vt "))
				++uvCoords;
			else if (line.StartsWith("vn "))
				++numNormals;
			else if (line.StartsWith("f "))
				++numFaces;
		}
	}
	int64 parseTime = t.GetMs();
	std::cout<<"\nInitial count-parse time: "<<parseTime;

	/// If bad, return.
	if (numFaces <= 0)
		return false;

	t.Start();

	// Find out how many numVertices are in the file.
	mesh->numVertices = numVertices;
	mesh->numUVs = uvCoords;
	// Find out how many numFaces are in the file.
	mesh->numFaces = numFaces;
	mesh->numNormals = numNormals;

	bool hadNormals = false;
	// Allocate the necessary numNormals
	if (numNormals > 0 ){
		hadNormals = true;
	}
	else {
		// Or as many as there are numFaces if we ahve to generate them!
		mesh->numNormals = numFaces;
	}


	/// allocate them arrays
	mesh->AllocateArrays();


	allocationTime = t.GetMs();
	std::cout<<"\nSecond allocationTime: "<<allocationTime;

 //   std::cout<<"\nMesh after allocation.";
//	mesh->PrintContents();


	/// Print some debug info
	std::cout<<"\n"<<filename<<" successfully read! \nParsing data:";
	std::cout<<"\n- "<<mesh->numVertices<<" numVertices";
	std::cout<<"\n- "<<mesh->numFaces<<" numFaces";
	std::cout<<"\n- "<<mesh->numUVs<<" numUVs";
	std::cout<<"\n- "<<mesh->numNormals<<" numNormals";

	
	t.Start();

	char row[128];
	memset(row, 0, 128);
	char * rPtr = &row[0];
	// Now do the actual loading from the raw data.
	char * ptr = data;

	int verticesRead = 0;
	int uvsRead = 0;
	int normalsRead = 0;
	int facesRead = 0;

	std::cout<<"\nParsing rows";

    /// Parse every row.. bettar!
    for (int i = 0; i < lines.Size(); ++i)
	{
		if (i % 10000 == 0)
			std::cout<<"\n"<<i<<" of "<<lines.Size()<<" lines parsed.";
		String line = lines[i];
	//	std::cout<<"\nParsing row: "<<line;
        // Split row into tokens
       	List<String> tokens = line.Tokenize(" ");
	//	std::cout<<"\nTokenized";
		if (tokens.Size() < 1)
			continue;
		String type = tokens[0];
	//	std::cout<<"\nType fetch o/o"<<line;
        // Read in vertices values and save them
		if (type.Contains("#"))
			continue;
        else if (type == "v"){
			if (tokens.Size() < 4)
				continue;
			mesh->vertices[verticesRead] = Vector3f(tokens[1].ParseFloat(),tokens[2].ParseFloat(),tokens[3].ParseFloat());
   //         std::cout<<"\nVertex parsed: "<<mesh->vertices[verticesRead]<< " (verticesRead: "<<verticesRead<<")";
            ++verticesRead;
        }
        // Read in vertices texture mapping coordinates and save them
        else if (type == "vt")
		{
			if (tokens.Size() < 3)
				continue;
            mesh->uvs[uvsRead][0] = tokens[1].ParseFloat();
            mesh->uvs[uvsRead][1] = tokens[2].ParseFloat();
            ++uvsRead;
        }
        // Read in vertices normals mapping coordinates and save them
        else if (type == "vn")
		{
			if (tokens.Size() < 4)
				continue;
            mesh->normals[normalsRead] = Vector3f(tokens[1].ParseFloat(),tokens[2].ParseFloat(),tokens[3].ParseFloat());
            ++normalsRead;
        }
        // Read in faces values and save them
        else if (type == "f")
		{
			int faceVertices = tokens.Size() - 1;
			MeshFace * faces = &mesh->faces[facesRead];
            // Allocate the faces number of vertexes depending on the vertexTokens found.
            faces->numVertices = faceVertices;
			faces->AllocateArrays();
            // Nullify
			memset(faces->vertices.GetArray(), 0, sizeof(unsigned int) * faceVertices);
			memset(faces->uvs.GetArray(), 0, sizeof(unsigned int) * faceVertices);
			memset(faces->normals.GetArray(), 0, sizeof(unsigned int) * faceVertices);
	
			// If more numFaces to add, take note of it now, since we want to triangulize everything? Eh? o.O
			if (faceVertices > 3){
                int facesToAdd = faceVertices - 3;
            }

			List<String> faceVertexTokens = tokens;
			bool success = faceVertexTokens.RemoveIndex(0, ListOption::RETAIN_ORDER);
			assert(success);
            
			// Go through all vertices tokens that were found and tokenize again...
			for	(int i = 0; i < faceVertexTokens.Size(); ++i)
			{
                // Extra care for files with just numVertices and numNormals..
                // Count slashes before the tokenizer enters any null-signs
                int slashes = 0;
				String faceVertexData = faceVertexTokens[i];
				slashes = faceVertexData.Count('/');
				List<String> faceVertexDataTokens = faceVertexData.Tokenize("/");
				/// Amount of "indices" found for this faces's vertices. That is, the indexes of this vertices' position, uvs and/or normals.
				int numFaceVertexDataTokens = faceVertexDataTokens.Size();
                // Regular numFaces without any spacing.
				if (slashes == numFaceVertexDataTokens - 1)
				{
					switch(numFaceVertexDataTokens){
                    case 3:
                        // Normal vertices
                        mesh->faces[facesRead].normals[i] = faceVertexDataTokens[2].ParseInt() - 1;	// -1 since they begin counting at 1!
                    case 2:
                        // UV Vertex
                        mesh->faces[facesRead].uvs[i] = faceVertexDataTokens[1].ParseInt() - 1;		// -1 since they begin counting at 1!
                    case 1:
                        // Carteesian vertices
                        mesh->faces[facesRead].vertices[i] = faceVertexDataTokens[0].ParseInt() - 1;	// -1 since they begin counting at 1!
                        break;
                    }
                }
                // Assume 2 slashes and just 2 tokens: vertices + normals
                else if (slashes == 2 && numFaceVertexDataTokens == 2){
                    // Normal vertices
                    mesh->faces[facesRead].normals[i] = faceVertexDataTokens[1].ParseInt() - 1;	// -1 since they begin counting at 1!
                    // Carteesian vertices
                    mesh->faces[facesRead].vertices[i] = faceVertexDataTokens[0].ParseInt() - 1;	// -1 since they begin counting at 1!
                }
    //            std::cout<<"\nmesh->faces[facesRead:"<<facesRead<<"].vertices[i]: "<<mesh->faces[facesRead].vertices[i];
                assert(mesh->faces[facesRead].vertices[i] < 3000000);

            } // End of parsing this faces.
            // If we didn't have any numNormals, generate them now!
            if (!hadNormals)
			{
                MeshFace * f = &mesh->faces[facesRead];
                Vector3f side1, side2;
                side1 = mesh->vertices[f->vertices[1]] - mesh->vertices[f->vertices[0]];
                side2 = mesh->vertices[f->vertices[2]] - mesh->vertices[f->vertices[0]];
                mesh->normals[facesRead] = side1.CrossProduct(side2).NormalizedCopy();
                // Link "all the numNormals" in the faces to this created one.
                for (int j = 0; j < faceVertices; ++j)
                    f->normals[j] = facesRead;
            }

            ++facesRead;
        }	// End of reading in faces
	}	// End of while-loop reading through all data

	/// Set source name at least
	mesh->name = filename;
	mesh->source = filename;

	parseTime = t.GetMs();
	std::cout<<"\nSecond major Parse time: "<<parseTime;

	/// Print some debug info
	std::cout<<"\n"<<mesh->source<<" successfully read! \nParsing data:";
	std::cout<<"\n- "<<mesh->numVertices<<" numVertices";
	std::cout<<"\n- "<<mesh->numFaces<<" numFaces";
	std::cout<<"\n- "<<mesh->numUVs<<" numUVs";
	std::cout<<"\n- "<<mesh->numNormals<<" numNormals";

	return true;
}

