// Emil Hedemalm
// 2013-07-21
// Mainly aimed to import animation if possibru.

#include "ColladaImporter.h"
#include "XML/XMLParser.h"
#include "XML/XMLElement.h"
#include "Mesh/Mesh.h"

#include "SkeletalAnimationNode.h"

#include "String/StringUtil.h"

ColladaImporter::ColladaImporter()
{
	up_axis = Y_UP;
	unit = 1.0f;
	parser = NULL;

	library_animations = NULL;
}


ColladaImporter::~ColladaImporter(){
	if (parser)
		delete parser;
	parser = NULL;
}

/// Attempts to load from file. All content will be stored internally until a build-command is issued.
bool ColladaImporter::Load(String fromFile)
{
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

	source = fromFile;
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

	// Quickly scan for geometry
	Xelement * geometries = parser->GetElement("library_geometries");
	if (geometries){
		geometryList = geometries->children;
		for (int i = 0; i < geometryList.Size(); ++i){
			Xelement * geometry = geometryList[i];
			Xarg * name = geometry->GetArgument("name");
			std::cout<<"\nFound geometry: "<<name->value;
		}
	}
	// .. animations
	library_animations = parser->GetElement("library_animations");
	if (library_animations)
	{
		animationList = library_animations->children;
		for (int i = 0; i < animationList.Size(); ++i)
		{
			Xelement * animation = animationList[i];
			Xarg * name = animation->GetArgument("id");
			std::cout<<"\nFound animation: "<<name->value;
		}
	}
	// .. controllers (e.g. bones)
	library_controllers = parser->GetElement("library_controllers");
	if (library_controllers)
	{
		controllerList = library_controllers->children;
		for (int i = 0; i < controllerList.Size(); ++i)
		{
			Xelement * controller = controllerList[i];
			Xarg * name = controller->GetArgument("name");
			std::cout<<"\nFound controller: "<<name->value;
		}
	}

	library_visual_scenes = parser->GetElement("library_visual_scenes");

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
Mesh * ColladaImporter::CreateMesh(String name)
{
	Mesh * mesh = new Mesh();
	Xelement * geo = GetGeometry(name);
	assert(geo);
	/// Find out what's needed in here to create stuff.
	Xelement * meshElement = geo->GetElement("mesh");
	assert(meshElement);
	mesh->source = source;
	mesh->name = name;

	/////////////////////////////////////////////////////////////////
	// Parse vertices!
	/////////////////////////////////////////////////////////////////
	Xelement * vertices = meshElement->GetElement("vertices");
	assert(vertices);
	if (vertices)
	{
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
		mesh->numVertices = vertexSource->GetElement("accessor")->GetArgument("count")->value.ParseInt();
		assert(mesh->numVertices > 0);

		// Set full after allocation.
		
		List<String> vertexDataTokens = floatArray->data.Tokenize(" ");
		List<float> vertexFloatList = StringListToFloatList(vertexDataTokens);
		mesh->vertices = Vector3f::FromFloatList(vertexFloatList, mesh->numVertices);
	}
	// Swap co-ordinates as needed.
	switch(up_axis)
	{
		case Z_UP:
		{
			for (int i = 0; i < mesh->vertices.Size(); ++i)
			{
				Vector3f v = mesh->vertices[i];
				mesh->vertices[i] = Vector3f(v[0], v[2], v[1]);
			}
			break;
		}		
		case Y_UP:
		{
			// Automatically compatible, do nothing.
			break;
		}
		default:
			assert(false);
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
	for (int i = 0; i < inputs.Size(); ++i)
	{
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
	// If it had normals, fetch 'em.
	if (hasNormals)
	{	
		Xelement * normalSourceElement = meshElement->GetElement("source", normalsSource);
		std::cout<<"\nFinding normal data...";
		Xelement * floatArray = normalSourceElement->GetElement("float_array");
		Xarg * numNormalsArg = floatArray->GetArgument("count");
		assert(numNormalsArg);
		mesh->numNormals = numNormalsArg->value.ParseInt();
		assert(mesh->numNormals > 0);
		std::cout<<"\nNormals to parse: "<<mesh->numNormals;
		/// o-o
		mesh->normals.Allocate(mesh->numNormals, true);
		List<String> normalDataTokens = floatArray->data.Tokenize(" ");
		assert(normalDataTokens.Size() == mesh->numNormals);
		int normalsParsed = 0;
		for (int i = 0; i < normalDataTokens.Size(); i += 3)
		{
			mesh->normals[normalsParsed][0] = normalDataTokens[i].ParseFloat();
			mesh->normals[normalsParsed][1] = normalDataTokens[i+1].ParseFloat();
			mesh->normals[normalsParsed][2] = normalDataTokens[i+2].ParseFloat();
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
	for (int i = 0; i < inputs.Size(); ++i)
	{
		Xelement * input = inputs[i];
		Xarg * semantic = input->GetArgument("semantic");
		if (semantic->value == "TEXCOORD")
		{
			Xarg * uvSource = input->GetArgument("source");
			uvsSource = uvSource->value;
			uvsSource.Remove("#");
			hasUVs = true;
			break;
		}
	}
	// If it had UV's, parse 'em!
	if (hasUVs)
	{	
		Xelement * uvSourceElement = meshElement->GetElement("source", uvsSource);
		std::cout<<"\nFinding normal data...";
		Xelement * floatArray = uvSourceElement->GetElement("float_array");
		Xarg * numUVsArg = floatArray->GetArgument("count");
		assert(numUVsArg);
		mesh->numUVs = numUVsArg->value.ParseInt();
		assert(mesh->numUVs > 0);
		std::cout<<"\nUVs to parse: "<<mesh->numUVs;
		
		mesh->uvs.Allocate(mesh->numUVs, true);
		
		List<String> uvDataTokens = floatArray->data.Tokenize(" ");
		assert(uvDataTokens.Size() == mesh->numUVs);
		int uvsParsed = 0;
		for (int i = 0; i < uvDataTokens.Size(); i += 2)
		{
			mesh->uvs[uvsParsed][0] = uvDataTokens[i].ParseFloat();
			mesh->uvs[uvsParsed][1] = uvDataTokens[i+1].ParseFloat();
			++uvsParsed;
		}
	}

	/////////////////////////////////////////////////////////////////
	// Parse/create faces!
	/////////////////////////////////////////////////////////////////
	List<XMLElement*> polyLists = meshElement->GetElements("polylist");

	int facesNeeded = 0;
	/// First gather how many faces we'll need in total for all poly lists..!
	for (int i = 0; i < polyLists.Size(); ++i)
	{
		Xelement * polyList = polyLists[i];
		int polysInThisPolyList = polyList->GetArgument("count")->value.ParseInt();
		facesNeeded += polysInThisPolyList;
		std::cout<<"\nPolylist "<<i<<" faces: "<<polysInThisPolyList;
	}

	std::cout<<"\nTotal faces needed: "<<facesNeeded;
	// Create faces, jaow.
	mesh->faces.Allocate(facesNeeded, true);
	mesh->numFaces = facesNeeded;

	// For each poly-list, create faces!
	int facesParsed = 0;
	for (int i = 0; i < polyLists.Size(); ++i)
	{
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
		for (int i = 0; i < polysInThisPolyList; ++i)
		{
			MeshFace * face = &mesh->faces[facesParsed];
			++facesParsed;
			face->numVertices = faceVertexCounts[i].ParseInt();
			assert(face->numVertices > 0 && face->numVertices < 20);
			assert(face->vertices.Size() == NULL);
			face->AllocateArrays();
			for (int i = 0; i < face->numVertices; ++i)
			{
				face->vertices[i] = polyArgList[polyArgListEnumerator].ParseInt();
				if (hasNormals)
				{
					face->normals[i] = polyArgList[polyArgListEnumerator+1].ParseInt();
				}
				if (hasUVs)
				{
					face->uvs[i] = polyArgList[polyArgListEnumerator+2].ParseInt();
				}

				/// The stride is usually the size of the input list. I.e. 3 for vertex/normal/UV, 2 for vertex/normal, etc.
				polyArgListEnumerator += inputs;
			}
		}
	}

	// Create skeleton tooo! o.o

	// Create skeleton!
	SkeletalAnimationNode * skeleton =  CreateSkeleton("root");
	if (skeleton)
	{
		mesh->skeleton = skeleton;
		// Load animations for the skeleton, if there are any?
		skeleton->LoadAnimations(animationList);



		// Get skin!
		Xelement * skin = library_controllers->GetElement("skin");
		// Load skinning data!
		Xelement * vertexWeightsElement = skin->GetElement("vertex_weights");
		List<String> vcount = vertexWeightsElement->GetElement("vcount")->data.Tokenize(" \n\t");
		// weights per vertex.
		mesh->weightsPerVertex = StringListToIntList(vcount);
		// 
		List<String> vertexWeightIndicesStr = vertexWeightsElement->GetElement("v")->data.Tokenize(" \n\t");
		List<int> vertexJointWeightIndices = StringListToIntList(vertexWeightIndicesStr);
		
		/// Fetch bind matrices.
		Xelement * jointsE = skin->GetElement("joints");
		String invBindMatrixSource = jointsE->GetElement("input", "semantic", "INV_BIND_MATRIX")->GetArgument("source")->value;
		invBindMatrixSource.Remove("#");
		Xelement * invBindMatrix = skin->GetElement("source", "id", invBindMatrixSource);
		int numMatrices = invBindMatrix->GetElement("accessor")->GetArgument("count")->value.ParseInt();
		List<float> bindPosesFloats = StringListToFloatList(invBindMatrix->GetElement("float_array")->data.Tokenize(" \n\t"));
		/// o.o Create matrices!
		List<Matrix4f> invBindPoseMatrices = Matrix4f::FromFloatList(bindPosesFloats, numMatrices, true);
		/// o.o
		mesh->invBindPoseMatrices = invBindPoseMatrices;
	
		String weightsSourceStr, jointNamesStr;

		// Fetch bind-shape matrix (relationship between skeleton and skin)
		mesh->bindShapeMatrix = Matrix4f::FromFloatList(StringListToFloatList(skin->GetElement("bind_shape_matrix")->data.Tokenize(" \t\n")), 1, true);

		// Fetch input semantics. Ensure that we are parsing the right way.
		List<Xelement*> vwInputs = vertexWeightsElement->GetElements("input");
		for (int i = 0; i < vwInputs.Size(); ++i)
		{
			Xelement * input = vwInputs[i];
			Xarg * semantic = input->GetArgument("semantic"), * offset = input->GetArgument("offset");
			switch(i)
			{
				case 0:
					assert(semantic->value == "JOINT" && offset->value == "0");
					jointNamesStr = input->GetArgument("source")->value;
					jointNamesStr.Remove("#");
					break;
				case 1:
					assert(semantic->value == "WEIGHT" && offset->value == "1");
					weightsSourceStr = input->GetArgument("source")->value;
					weightsSourceStr.Remove("#");
					break;
				default:
					assert(false);
			}
		}
		// Fetch names.
		Xelement * jointsElement = skin->GetElement("source", "id", jointNamesStr);
		List<String> names = jointsElement->GetElement("Name_array")->data.Tokenize(" \t\n");
		skeleton->AssignBoneIndices(names);

		/// Fetch weights.
		Xelement * weightsSource = skin->GetElement("source", "id", weightsSourceStr);		
		List<float> vertexWeights = StringListToFloatList(weightsSource->GetElement("float_array")->data.Tokenize(" \n\t"));
	
		// For each vertex..
		int vertexJointWeightIndex = 0;
		int maxWeightsPerVertex = 0;

		int numVertexWeights = vertexWeightsElement->GetArgument("count")->value.ParseInt();
		for (int vertexIndex = 0; vertexIndex < numVertexWeights; ++vertexIndex)
		{
			// Add a new list of weights.
			List<VertexWeight> weights;
			int weightsForThisVertex = mesh->weightsPerVertex[vertexIndex];
			if (weightsForThisVertex > maxWeightsPerVertex)
				maxWeightsPerVertex = weightsForThisVertex;
			// For each weight per vertex
			for (int vertexWeightIndex = 0; vertexWeightIndex < weightsForThisVertex; ++vertexWeightIndex)
			{
					int jointIndex = vertexJointWeightIndices[vertexJointWeightIndex];
					int weightIndex = vertexJointWeightIndices[vertexJointWeightIndex+1];
					float weight = vertexWeights[weightIndex];
					VertexWeight vw;
					vw.boneIndex = jointIndex;
					vw.weight = weight;
					weights.Add(vw);
				//	std::cout<<"\nVertexWeight: "<<vw.weight;
					vertexJointWeightIndex += 2;
			}
			mesh->vertexWeights.Add(weights);
		}
		mesh->maxWeightsPerVertex = maxWeightsPerVertex;
	}
	

	return mesh;
//	return NULL;
}

/// Creates a skeleton (including all animation) based on the underlying nodes attached to the node of given name (must be defined in the visual scene of the collada file)
SkeletalAnimationNode * ColladaImporter::CreateSkeleton(String nodeName)
{
	// Find the element.
	Xelement * root = library_visual_scenes->GetElement("node", "name", nodeName);
	if (!root)
	{
		std::cout<<"\nNode with given name \'"<<nodeName<<"\'could not be found.";
		return NULL;
	}
	Bone * newSkeleton = new Bone();
	newSkeleton->ParseCollada(root);
	return newSkeleton;
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

