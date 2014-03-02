// Emil Hedemalm är trött.
// 2013-07-03

#include "ObjReader.h"
#include <iostream>
#include <cstring>

bool ObjReader::ReadObj(const char * filename, Mesh * mesh){

	char * data;
	// Size of read data
	int size;

	int vertices = 0;
	int uvCoords = 0;
	int normals = 0;
	int faces = 0;

 //   std::cout<<"\nMesh before loading!";
//	mesh->PrintContents();

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

		// Then go to beginning again, and count occurences of "v ", "vt " and "f "!
		std::stringstream ss(data);
		char line[128];
		while (ss.good()){
			ss.getline(line, 128);
			if (strncmp(line, "v ", 2) == 0)
				++vertices;
			else if (strncmp(line, "vt ", 2) == 0)
				++uvCoords;
			else if (strncmp(line, "vn ", 2) == 0)
				++normals;
			else if (strncmp(line, "f ", 2) == 0)
				++faces;
		}


		// Then close the stream
		fileStream.close();
	} catch (...){
		return false;
	}

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

 //   std::cout<<"\nMesh after allocation.";
//	mesh->PrintContents();


	/// Print some debug info
	std::cout<<"\n"<<mesh->source<<" successfully read! \nParsing data:";
	std::cout<<"\n- "<<mesh->vertices<<" vertices";
	std::cout<<"\n- "<<mesh->faces<<" faces";
	std::cout<<"\n- "<<mesh->uvs<<" uvs";
	std::cout<<"\n- "<<mesh->normals<<" normals";

    std::stringstream sstream(data);
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
    while (sstream.good()){
        sstream.getline(row, 128);

  //      std::cout<<"\nRow to evaluate: "<<row;
  //      std::cout<<"\nptr: (int)"<<(int)*ptr<<" (char)"<<(char)*ptr;

        // Split row into tokens
        char delim[10] = " ";
        int tokensFound = 0;
        std::istringstream in(row);
        std::string type;
        in >> type;
   //     std::cout<<"\nTokenized string? type: "<<type;

        // Read in vertex values and save them
        if (type.compare("v")== 0){
            float x, y, z;
            in >> x;
            in >> y;
            in >> z;
            mesh->vertex[verticesRead] = Vector3f(x,y,z);
   //         std::cout<<"\nVertex parsed: "<<mesh->vertex[verticesRead]<< " (verticesRead: "<<verticesRead<<")";
            ++verticesRead;
        }
        // Read in vertex texture mapping coordinates and save them
        else if (type.compare("vt")== 0){
            float u, v;
            in >> u;
            in >> v;
            mesh->u[uvsRead] = u;
            mesh->v[uvsRead] = v;
    //        std::cout<<"\nUV parsed: "<<u<<", "<<v<<" (uvRead: "<<uvsRead<<")";
            ++uvsRead;
        }
        // Read in vertex normal mapping coordinates and save them
        else if (type.compare("vn")== 0){
            float x, y, z;
            in >> x;
            in >> y;
            in >> z;
            mesh->normal[normalsRead] = Vector3f(x,y,z);
     //       std::cout<<"\nNormal parsed: "<<mesh->normal[normalsRead]<< " (normalsRead: "<<normalsRead<<")";
            ++normalsRead;
        }
        // Read in face values and save them
        else if (type.compare("f")== 0){

            char buf[128];
            in.getline(buf,128);

            // Split row into tokens
            char * vertexTokens[10]; // Hopefully maximum of decagon polygons...
            char delim[10] = " ";
            char * tok;
            int vTokensFound = 0;
            vertexTokens[vTokensFound++] = strtok(buf, delim);		// Tokenize first delimiter
            while(tok = strtok(NULL, delim))				// Continue until it returns null
                vertexTokens[vTokensFound++] = tok;

            // Allocate the face number of vertexes depending on the vertexTokens found.
            mesh->face[facesRead].numVertices = vTokensFound;
            mesh->face[facesRead].vertex = new unsigned int[vTokensFound];
            mesh->face[facesRead].uv = new unsigned int[vTokensFound];
            mesh->face[facesRead].normal = new unsigned int[vTokensFound];
            memset(mesh->face[facesRead].vertex, 0, sizeof(unsigned int) * vTokensFound);
            memset(mesh->face[facesRead].uv, 0, sizeof(unsigned int) * vTokensFound);
            memset(mesh->face[facesRead].normal, 0, sizeof(unsigned int) * vTokensFound);

      //      std::cout<<"\nFace found with "<<vTokensFound<<" vertices.";

            if (vTokensFound > 3){
                int facesToAdd = vTokensFound - 3;
                if (facesToAdd > 4){
       //             std::cout<<"\nFace vertex amount very large("<<(int)vTokensFound<<"), printing debug info:";
         //           assert(vTokensFound - 3 < 5);
                }
            }

            // Go through all vertex tokens that were found and tokenize again...
            for	(int i = 0; i < vTokensFound; ++i){

                // Extra care for files with just vertices and normals..
                // Count slashes before the tokenizer enters any null-signs
                int slashes = 0;
                for (int j = 0; j < (int)strlen(vertexTokens[i]); ++j){
                    if (vertexTokens[i][j] == '/'){
                        ++slashes;
                    }
                }

                // Tokenize the face vertex indices
                char * partTokens[3];
                char delim2[3] = "/";
                int pvTokensFound = 0;
                partTokens[pvTokensFound++] = strtok(vertexTokens[i], delim2);		// Tokenize first delimiter
                while(tok = strtok(NULL, delim2))
                    partTokens[pvTokensFound++] = tok;


                // Regular faces without any spacing.
                if (slashes == pvTokensFound - 1){
                    switch(pvTokensFound){
                    case 3:
                        // Normal vertex
                        mesh->face[facesRead].normal[i] = atoi(partTokens[2]) - 1;	// -1 since they begin counting at 1!
                    case 2:
                        // UV Vertex
                        mesh->face[facesRead].uv[i] = atoi(partTokens[1]) - 1;		// -1 since they begin counting at 1!
                    case 1:
                        // Carteesian vertex
                        mesh->face[facesRead].vertex[i] = atoi(partTokens[0]) - 1;	// -1 since they begin counting at 1!
                        break;
                    }
                }
                // Assume 2 slashes and just 2 tokens: vertex + normal
                else if (slashes == 2 && pvTokensFound == 2){
                    // Normal vertex
                    mesh->face[facesRead].normal[i] = atoi(partTokens[1]) - 1;	// -1 since they begin counting at 1!
                    // Carteesian vertex
                    mesh->face[facesRead].vertex[i] = atoi(partTokens[0]) - 1;	// -1 since they begin counting at 1!
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
                for (int j = 0; j < vTokensFound; ++j)
                    f->normal[j] = facesRead;
            }

            ++facesRead;
        }	// End of reading in face
					// Check if we need to evaluate it at all
        memset(row, 0, 128);

	}	// End of while-loop reading through all data

	//std::cout<<"\nMesh after Parsing!";
//	mesh->PrintContents();

	/// Set source name at least
	mesh->name = filename;
	mesh->source = filename;

	/// Print some debug info
	std::cout<<"\n"<<mesh->source<<" successfully read! \nParsing data:";
	std::cout<<"\n- "<<mesh->vertices<<" vertices";
	std::cout<<"\n- "<<mesh->faces<<" faces";
	std::cout<<"\n- "<<mesh->uvs<<" uvs";
	std::cout<<"\n- "<<mesh->normals<<" normals";

	return true;
}

