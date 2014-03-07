// Emil Hedemalm
// 2013-07-03
// Swapped out std:: class usage for own one in order to handle new .obj files that were failing to parse.

#include "ObjReader.h"
#include <iostream>
#include <cstring>
#include "Timer/Timer.h"

bool ObjReader::ReadObj(const char * filename, Mesh * mesh)
{
	std::cout<<"\n\nReadObj called for file: "<<filename;
	Timer t;
	t.Start();
	/// Make sure the mesh doesn't already contain data?
	char * data;
	// Size of read data
	int size;

	int vertices = 0;
	int uvCoords = 0;
	int normals = 0;
	int faces = 0;

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
	int allocationTime = t.GetMs();
	std::cout<<"\nFile load raw data buffer allocationTime: "<<allocationTime;


	t.Start();
	/// Try with our own string-class too.
	String stringData = data;
	List<String> lines = stringData.GetLines();
	int time = t.GetMs();
	std::cout<<"\nAllocating stringData and List<String> lines out of the raw data: "<<time;
	t.Start();
	/// Now delete the data, too.
	delete[] data;
	data = NULL;
	int deallocationTime = t.GetMs();
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
				++vertices;
			else if (line.StartsWith("vt "))
				++uvCoords;
			else if (line.StartsWith("vn "))
				++normals;
			else if (line.StartsWith("f "))
				++faces;
		}
	}
	int parseTime = t.GetMs();
	std::cout<<"\nInitial count-parse time: "<<parseTime;

	/// If bad, return.
	if (faces <= 0)
		return false;

	t.Start();

	// Find out how many vertices are in the file.
	mesh->vertices = vertices;
	// ... allocate vertex array
	mesh->vertex = new Vector3f[vertices];

	// Allocate the necessary uvCoordinates too.
	mesh->uvs = uvCoords;
	mesh->u = new float[uvCoords];
	mesh->v = new float[uvCoords];

	// Find out how many faces are in the file.
	mesh->faces = faces;
	// ... allocate face array.
	mesh->face = new MeshFace[faces];

	bool hadNormals = false;
	// Allocate the necessary normals
	if (normals > 0 ){
		mesh->normals = normals;
		mesh->normal = new Vector3f[normals];
		hadNormals = true;
	}
	else {
		// Or as many as there are faces if we ahve to generate them!
		mesh->normal = new Vector3f[faces];
	}
	allocationTime = t.GetMs();
	std::cout<<"\nSecond allocationTime: "<<allocationTime;

 //   std::cout<<"\nMesh after allocation.";
//	mesh->PrintContents();


	/// Print some debug info
	std::cout<<"\n"<<mesh->source<<" successfully read! \nParsing data:";
	std::cout<<"\n- "<<mesh->vertices<<" vertices";
	std::cout<<"\n- "<<mesh->faces<<" faces";
	std::cout<<"\n- "<<mesh->uvs<<" uvs";
	std::cout<<"\n- "<<mesh->normals<<" normals";

	
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

    /// Parse every row.. bettar!
    for (int i = 0; i < lines.Size(); ++i){
		String line = lines[i];
        // Split row into tokens
       	List<String> tokens = line.Tokenize(" ");
		String type = tokens[0];
        // Read in vertex values and save them
		if (type.Contains("#"))
			continue;
        else if (type == "v"){
			if (tokens.Size() < 4)
				continue;
			mesh->vertex[verticesRead] = Vector3f(tokens[1].ParseFloat(),tokens[2].ParseFloat(),tokens[3].ParseFloat());
   //         std::cout<<"\nVertex parsed: "<<mesh->vertex[verticesRead]<< " (verticesRead: "<<verticesRead<<")";
            ++verticesRead;
        }
        // Read in vertex texture mapping coordinates and save them
        else if (type == "vt")
		{
			if (tokens.Size() < 3)
				continue;
            mesh->u[uvsRead] = tokens[1].ParseFloat();
            mesh->v[uvsRead] = tokens[2].ParseFloat();
            ++uvsRead;
        }
        // Read in vertex normal mapping coordinates and save them
        else if (type == "vn")
		{
			if (tokens.Size() < 4)
				continue;
            mesh->normal[normalsRead] = Vector3f(tokens[1].ParseFloat(),tokens[2].ParseFloat(),tokens[3].ParseFloat());
            ++normalsRead;
        }
        // Read in face values and save them
        else if (type == "f")
		{
			int faceVertices = tokens.Size() - 1;
            // Allocate the face number of vertexes depending on the vertexTokens found.
            mesh->face[facesRead].numVertices = faceVertices;
            mesh->face[facesRead].vertex = new unsigned int[faceVertices];
            mesh->face[facesRead].uv = new unsigned int[faceVertices];
            mesh->face[facesRead].normal = new unsigned int[faceVertices];
            // Nullify
			memset(mesh->face[facesRead].vertex, 0, sizeof(unsigned int) * faceVertices);
            memset(mesh->face[facesRead].uv, 0, sizeof(unsigned int) * faceVertices);
            memset(mesh->face[facesRead].normal, 0, sizeof(unsigned int) * faceVertices);
	
			// If more faces to add, take note of it now, since we want to triangulize everything? Eh? o.O
			if (faceVertices > 3){
                int facesToAdd = faceVertices - 3;
            }

			List<String> faceVertexTokens = tokens;
			bool success = faceVertexTokens.RemoveIndex(0, ListOption::RETAIN_ORDER);
			assert(success);
            // Go through all vertex tokens that were found and tokenize again...
			for	(int i = 0; i < faceVertexTokens.Size(); ++i)
			{
                // Extra care for files with just vertices and normals..
                // Count slashes before the tokenizer enters any null-signs
                int slashes = 0;
				String faceVertexData = faceVertexTokens[i];
				slashes = faceVertexData.Count('/');
				List<String> faceVertexDataTokens = faceVertexData.Tokenize("/");
				/// Amount of "indices" found for this face's vertex. That is, the indexes of this vertex' position, uv and/or normal.
				int numFaceVertexDataTokens = faceVertexDataTokens.Size();
                // Regular faces without any spacing.
				if (slashes == numFaceVertexDataTokens - 1)
				{
					switch(numFaceVertexDataTokens){
                    case 3:
                        // Normal vertex
                        mesh->face[facesRead].normal[i] = faceVertexDataTokens[2].ParseInt() - 1;	// -1 since they begin counting at 1!
                    case 2:
                        // UV Vertex
                        mesh->face[facesRead].uv[i] = faceVertexDataTokens[1].ParseInt() - 1;		// -1 since they begin counting at 1!
                    case 1:
                        // Carteesian vertex
                        mesh->face[facesRead].vertex[i] = faceVertexDataTokens[0].ParseInt() - 1;	// -1 since they begin counting at 1!
                        break;
                    }
                }
                // Assume 2 slashes and just 2 tokens: vertex + normal
                else if (slashes == 2 && numFaceVertexDataTokens == 2){
                    // Normal vertex
                    mesh->face[facesRead].normal[i] = faceVertexDataTokens[1].ParseInt() - 1;	// -1 since they begin counting at 1!
                    // Carteesian vertex
                    mesh->face[facesRead].vertex[i] = faceVertexDataTokens[0].ParseInt() - 1;	// -1 since they begin counting at 1!
                }
    //            std::cout<<"\nmesh->face[facesRead:"<<facesRead<<"].vertex[i]: "<<mesh->face[facesRead].vertex[i];
                assert(mesh->face[facesRead].vertex[i] < 3000000);

            } // End of parsing this face.
            // If we didn't have any normals, generate them now!
            if (!hadNormals){
                MeshFace * f = &mesh->face[facesRead];
                Vector3f side1, side2;
                side1 = mesh->vertex[f->vertex[1]] - mesh->vertex[f->vertex[0]];
                side2 = mesh->vertex[f->vertex[2]] - mesh->vertex[f->vertex[0]];
                mesh->normal[facesRead] = side1.CrossProduct(side2).NormalizedCopy();
                // Link "all the normals" in the face to this created one.
                for (int j = 0; j < faceVertices; ++j)
                    f->normal[j] = facesRead;
            }

            ++facesRead;
        }	// End of reading in face
	}	// End of while-loop reading through all data

	/// Set source name at least
	mesh->name = filename;
	mesh->source = filename;

	parseTime = t.GetMs();
	std::cout<<"\nSecond major Parse time: "<<parseTime;

	/// Print some debug info
	std::cout<<"\n"<<mesh->source<<" successfully read! \nParsing data:";
	std::cout<<"\n- "<<mesh->vertices<<" vertices";
	std::cout<<"\n- "<<mesh->faces<<" faces";
	std::cout<<"\n- "<<mesh->uvs<<" uvs";
	std::cout<<"\n- "<<mesh->normals<<" normals";

	return true;
}

