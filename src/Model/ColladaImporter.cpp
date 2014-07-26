// Emil Hedemalm
// 2013-07-21
// Mainly aimed to import animation if possibru.

#include "ColladaImporter.h"
#include "XML/XMLParser.h"
#include "XML/XMLElement.h"
#include "Mesh.h"

ColladaImporter::ColladaImporter(){
	up_axis = Y_UP;
	unit = 1.0f;
	parser = NULL;
}


ColladaImporter::~ColladaImporter(){
	if (parser)
		delete parser;
	parser = NULL;
}

/// Attempts to load from file. All content will be stored internally until a build-command is issued.
bool ColladaImporter::Load(String fromFile){
	if (parser){
		assert(false && "Already loaded");
		return true;
	}
	parser = new XMLParser();
	bool result = parser->Read(fromFile);
	if (!result)
		return false;
	/// Parse the shit.
	result = parser->Parse();
	if (!result)
		return false;

	/// Find out what we've got inside..!

	// Begin with basics
	Xelement * vendor = parser->GetElement("authoring_tool");
	if (vendor){
		authoring_tool = vendor->data;
		std::cout<<"\nAuthoring tool: "<<vendor->data;
	}

	Xelement * unitElement = parser->GetElement("unit");
	if (unitElement){
		Xarg * meters = unitElement->GetArgument("meter");
		assert(meters);
		unit = meters->value.ParseFloat();
		assert(unit > 0);
	}

	Xelement * upAxis = parser->GetElement("up_axis"); 
	if (upAxis){
		String upAxisData = upAxis->data;
		if (upAxisData == "Z_UP")
			up_axis = Z_UP;
		else if (upAxisData == "Y_UP")
			up_axis = Y_UP;
		else {
			assert(false && "Undefined axis up, please look hierrre in cooooude.");
		}
	}

	// Wosh.
	Xelement * geometries = parser->GetElement("library_geometries");
	if (geometries){
		geometryList = geometries->children;
		for (int i = 0; i < geometryList.Size(); ++i){
			Xelement * geometry = geometryList[i];
			Xarg * name = geometry->GetArgument("name");
			std::cout<<"\nFound geometry: "<<name->value;
		}
	}



	/// Required for exporters..?
	// Translation, Scaling, Rotation, Parenting, Static objects, Animated objects, Skewing, Transparency/Reflectivity, Texture-mapping method..?
	// Transform w/o Geometry, RGB textures, RGBA textures, Baked procedural texture coordinates..?, Common profile material (PHONG, LAMBERT, etc)
	// Per-face material
	// Vertex texture coordinates, normals, binormals, tangents, UV-coordinates, colors, custom vertex attributes
	// Animation...
	// Scene data, empty nodes, cameras, spotlights, directional lights, point lights, ambient lights, 
	// Triangle lists, polygon lists, baked matrices, matrices

	return true;
}

/// Returns a list of all data that was parsed.
List<String> ColladaImporter::Data(){
	List<String> data;
	return data;
}
List<String> ColladaImporter::Meshes(){
	List<String> data;
	return data;
}
List<String> ColladaImporter::Geometries(){
	List<String> g;
	for (int i = 0; i < geometryList.Size(); ++i){
		Xelement * geo = geometryList[i];
		Xarg * arg = geo->GetArgument("name");
		assert(arg);
		g.Add(arg->value);
	}
	return g;
}


/// Creates the named mesh (a reference must exist within the parsed data).
Mesh * ColladaImporter::CreateMesh(String name){
	Mesh * mesh = new Mesh();
	Xelement * geo = GetGeometry(name);
	assert(geo);
	/// Find out what's needed in here to create stuff.
	Xelement * meshElement = geo->GetElement("mesh");
	assert(meshElement);
	
	/////////////////////////////////////////////////////////////////
	// Parse vertices!
	/////////////////////////////////////////////////////////////////
	Xelement * vertices = meshElement->GetElement("vertices");
	assert(vertices);
	if (vertices){
		std::cout<<"\nFinding vertex data...";
		Xelement * vertexInput = vertices->GetElement("input");
		assert(vertexInput);
		Xarg * semantic = vertexInput->GetArgument("semantic");
		assert(semantic);
		std::cout<<"\nSemantic: "<<semantic->value;
		Xarg * source = vertexInput->GetArgument("source");
		assert(source);
		String vertexSourceName = source->value;
		std::cout<<"\nSource: "<<vertexSourceName;
		vertexSourceName.Remove("#");
		Xelement * vertexSource = meshElement->GetElement("source", vertexSourceName);
		assert(vertexSource);
		Xelement * floatArray = vertexSource->GetElement("float_array");
		Xarg * numVertsArg = floatArray->GetArgument("count");
		assert(numVertsArg);
		mesh->vertices = numVertsArg->value.ParseInt();
		assert(mesh->vertices > 0);
		mesh->vertex = new Vector3f[mesh->vertices];
		
		List<String> vertexDataTokens = floatArray->data.Tokenize(" ");
		assert(vertexDataTokens.Size() == mesh->vertices);
		int vertexPositionsParsed = 0;
		for (int i = 0; i < vertexDataTokens.Size(); i += 3){
			mesh->vertex[vertexPositionsParsed].x = vertexDataTokens[i].ParseFloat();
			mesh->vertex[vertexPositionsParsed].y = vertexDataTokens[i+1].ParseFloat();
			mesh->vertex[vertexPositionsParsed].z = vertexDataTokens[i+2].ParseFloat();
			++vertexPositionsParsed;
		}
	}
	switch(up_axis){
		case Z_UP:{
			for (int i = 0; i < mesh->vertices; ++i){
				Vector3f v = mesh->vertex[i];
				mesh->vertex[i] = Vector3f(v.x, v.z, v.y);
			}
			break;
		}		
	}

	/////////////////////////////////////////////////////////////////
	// Parse normals!
	/////////////////////////////////////////////////////////////////
	// Grab a random polylist in order to find a source-identifier for the normals :)
	Xelement * tempPL = meshElement->GetElement("polylist");
	assert(tempPL);
	List<Xelement*> inputs = tempPL->GetElements("input");
	String normalsSource;
	bool hasNormals = false;
	for (int i = 0; i < inputs.Size(); ++i){
		Xelement * input = inputs[i];
		Xarg * semantic = input->GetArgument("semantic");
		if (semantic->value == "NORMAL"){
			Xarg * normalSource = input->GetArgument("source");
			normalsSource = normalSource->value;
			normalsSource.Remove("#");
			hasNormals = true;
			break;
		}
	}
	if (hasNormals){	
		Xelement * normalSourceElement = meshElement->GetElement("source", normalsSource);
		std::cout<<"\nFinding normal data...";
		Xelement * floatArray = normalSourceElement->GetElement("float_array");
		Xarg * numNormalsArg = floatArray->GetArgument("count");
		assert(numNormalsArg);
		mesh->normals = numNormalsArg->value.ParseInt();
		assert(mesh->normals > 0);
		std::cout<<"\nNormals to parse: "<<mesh->normals;
		mesh->normal = new Vector3f[mesh->normals];
		List<String> normalDataTokens = floatArray->data.Tokenize(" ");
		assert(normalDataTokens.Size() == mesh->normals);
		int normalsParsed = 0;
		for (int i = 0; i < normalDataTokens.Size(); i += 3){
			mesh->normal[normalsParsed].x = normalDataTokens[i].ParseFloat();
			mesh->normal[normalsParsed].y = normalDataTokens[i+1].ParseFloat();
			mesh->normal[normalsParsed].z = normalDataTokens[i+2].ParseFloat();
			++normalsParsed;
		}
	}

	/////////////////////////////////////////////////////////////////
	// Parse UV-coordinates!!
	/////////////////////////////////////////////////////////////////
	// Grab a random polylist in order to find a source-identifier for the normals :)
	assert(tempPL);
	String uvsSource;
	bool hasUVs = false;
	for (int i = 0; i < inputs.Size(); ++i){
		Xelement * input = inputs[i];
		Xarg * semantic = input->GetArgument("semantic");
		if (semantic->value == "TEXCOORD"){
			Xarg * uvSource = input->GetArgument("source");
			uvsSource = uvSource->value;
			uvsSource.Remove("#");
			hasUVs = true;
			break;
		}
	}
	if (hasUVs){	
		Xelement * uvSourceElement = meshElement->GetElement("source", uvsSource);
		std::cout<<"\nFinding normal data...";
		Xelement * floatArray = uvSourceElement->GetElement("float_array");
		Xarg * numUVsArg = floatArray->GetArgument("count");
		assert(numUVsArg);
		mesh->uvs = numUVsArg->value.ParseInt();
		assert(mesh->uvs > 0);
		std::cout<<"\nUVs to parse: "<<mesh->uvs;
		mesh->uv = new Vector2f[mesh->uvs];
		List<String> uvDataTokens = floatArray->data.Tokenize(" ");
		assert(uvDataTokens.Size() == mesh->uvs);
		int uvsParsed = 0;
		for (int i = 0; i < uvDataTokens.Size(); i += 2){
			mesh->uv[uvsParsed].x = uvDataTokens[i].ParseFloat();
			mesh->uv[uvsParsed].y = uvDataTokens[i+1].ParseFloat();
			++uvsParsed;
		}
	}

	/////////////////////////////////////////////////////////////////
	// Parse/create faces!
	/////////////////////////////////////////////////////////////////
	List<XMLElement*> polyLists = meshElement->GetElements("polylist");

	int facesNeeded = 0;
	/// First gather how many faces we'll need in total for all poly lists..!
	for (int i = 0; i < polyLists.Size(); ++i){
		Xelement * polyList = polyLists[i];
		int polysInThisPolyList = polyList->GetArgument("count")->value.ParseInt();
		facesNeeded += polysInThisPolyList;
		std::cout<<"\nPolylist "<<i<<" faces: "<<polysInThisPolyList;
	}

	std::cout<<"\nTotal faces needed: "<<facesNeeded;
	// Create faces, jaow.
	mesh->face = new MeshFace[facesNeeded];
	mesh->faces = facesNeeded;

	// For each poly-list, create faces!
	int facesParsed = 0;
	for (int i = 0; i < polyLists.Size(); ++i){
		Xelement * polyList = polyLists[i];
		int polysInThisPolyList = polyList->GetArgument("count")->value.ParseInt();
		List<Xelement*> inputList = polyList->GetElements("input");
		int inputs = inputList.Size();
		std::cout<<"\nInputs: "<<inputs;
		
		/// Parse face vertex counts now
		Xelement * faceVCountElement = polyList->GetElement("vcount");
		List<String> faceVertexCounts = faceVCountElement->data.Tokenize(" ");
		assert(polysInThisPolyList == faceVertexCounts.Size());
		
		/// Parse the polygon list now too.
		Xelement * pList = polyList->GetElement("p");
		List<String> polyArgList = pList->data.Tokenize(" ");

		// Create le faces.
		int polyArgListEnumerator = 0;
		for (int i = 0; i < polysInThisPolyList; ++i){
			MeshFace * face = &mesh->face[facesParsed];
			++facesParsed;
			face->numVertices = faceVertexCounts[i].ParseInt();
			assert(face->numVertices > 0 && face->numVertices < 20);
			assert(face->vertex == NULL);
			face->vertex = new unsigned int[face->numVertices];
			if (hasNormals)
				face->normal = new unsigned int[face->numVertices];
			if (hasUVs)
				face->uv = new unsigned int[face->numVertices];

			for (int i = 0; i < face->numVertices; ++i){
				face->vertex[i] = polyArgList[polyArgListEnumerator].ParseInt();
				if (hasNormals){
					face->normal[i] = polyArgList[polyArgListEnumerator+1].ParseInt();
				}
				if (hasUVs){
					face->uv[i] = polyArgList[polyArgListEnumerator+2].ParseInt();
				}

				/// The stride is usually the size of the input list. I.e. 3 for vertex/normal/UV, 2 for vertex/normal, etc.
				polyArgListEnumerator += inputs;
			}
		}
	}
	return mesh;
}

// Private functions, utility and stuff.
XMLElement * ColladaImporter::GetGeometry(String byName){
	List<String> g;
	for (int i = 0; i < geometryList.Size(); ++i){
		Xelement * geo = geometryList[i];
		Xarg * arg = geo->GetArgument("name");
		assert(arg);
		if (arg->value == byName)
			return geo;
	}
	return NULL;
}